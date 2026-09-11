import org.apache.storm.trident.TridentTopology
import org.apache.storm.trident.operation.{BaseFunction, TridentCollector}
import org.apache.storm.trident.tuple.TridentTuple
import org.apache.storm.tuple.{Fields, Values}
import org.apache.storm.{Config, LocalCluster}
import redis.clients.jedis.Jedis
import java.sql.DriverManager

// 1. Custom Spout pulling streaming parameters from Redis pipeline
class RedisMetricsSpout(host: String, port: Int) extends org.apache.storm.trident.spout.IBatchSpout {
  override def open(conf: java.util.Map[_, _], context: org.apache.storm.task.TopologyContext): Unit = {}
  override def emitBatch(batchId: Long, collector: TridentCollector): Unit = {
    val jedis = new Jedis(host, port)
    // Non-blocking pop from the streaming metric queue
    val data = jedis.rpop("storm_metrics")
    jedis.close()
    if (data != null) {
      // Basic split regex for simplified demo payload parsing
      val chef = data.split("\"chef\":\"")(1).split("\"")(0)
      val latency = data.split("\"latency\":")(1).split("}")(0).toDouble
      collector.emit(new Values(chef, Double.box(latency)))
    }
  }
  override def ack(batchId: Long): Unit = {}
  override def close(): Unit = {}
  override def getOutputFields: Fields = new Fields("chef", "latency")
}

// 2. Persistent SQLite DB Transaction Sink Function
class SQLiteSinkFunction extends BaseFunction {
  override def execute(tuple: TridentTuple, collector: TridentCollector): Unit = {
    val chef = tuple.getStringByField("chef")
    val latency = tuple.getDoubleByField("latency")

    val url = "jdbc:sqlite:competition.db"
    val conn = DriverManager.getConnection(url)

    // Auto-generate target schema structures dynamically on runtime access
    val createTableSql =
      """CREATE TABLE IF NOT EXISTS scoreboard (
        | id INTEGER PRIMARY KEY AUTOINCREMENT,
        | chef TEXT,
        | latency REAL
        |);""".stripMargin
    val stmt = conn.createStatement()
    stmt.execute(createTableSql)

    // Append the latest asynchronous processing match result logs
    val insertSql = "INSERT INTO scoreboard(chef, latency) VALUES(?, ?);"
    val pstmt = conn.prepareStatement(insertSql)
    pstmt.setString(1, chef)
    pstmt.setDouble(2, latency)
    pstmt.executeUpdate()

    pstmt.close()
    conn.close()
  }
}

object StormCompetitionTopology {
  def main(args: Array[String]): Unit = {
    val topology = new TridentTopology()
    val spout = new RedisMetricsSpout("127.0.0.1", 6379)

    topology.newStream("redis-metrics-stream", spout)
      .each(new Fields("chef", "latency"), new SQLiteSinkFunction(), new Fields())

    val config = new Config()
    val cluster = new LocalCluster()
    cluster.submitTopology("DistributedCompetitionTracker", config, topology.build())
  }
}
