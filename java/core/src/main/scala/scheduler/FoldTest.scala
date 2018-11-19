package scheduler

object FoldTest {

  def main(args: Array[String]): Unit = {
    val xs = 1 to 100
    val x = 10
    val init = (0,0)
    val ret = xs.toList.foldLeft(init) { case ((a,b),c) => (a + c, b + c * c) }
    println(s"ret is $ret")
  }
}
