
trait IHandle {
  def handleName : String
  def grab(): Unit
  def swing(): Unit
}

trait IHead {
  def headName : String
  def sharpen() : Unit
  def clean() : Unit
}

trait SteelHead extends IHead { self : IHandle =>


  override def headName: String = "Steel"

  override def sharpen(): Unit = {
    println("SteelHead sharpen")
    swing()
  }

  override def clean(): Unit = {
    println("SteelHead clean")
    println("all Done")
  }
}

trait BrassHead extends IHead { self : IHandle =>
  override def headName: String = "Brass"
  override def sharpen(): Unit = {
    println("Brasshead sharpen")
    swing()
  }

  override def clean(): Unit = {
    println("Brasshead clean")
    println("all Done")
  }
}


trait WoodHandle extends IHandle { self : IHead =>
  override def handleName: String = "Wood"

  override def grab(): Unit = {
    println("Woodhandle grab")
    sharpen()
  }

  override def swing(): Unit = {
    println("WoodHandle swing")
    clean()
  }
}


trait PlasticHandle extends IHandle { self : IHead =>
  override def handleName: String = "Plastic"

  override def grab(): Unit = {
    println("Plastichandle grab")
    sharpen()
  }

  override def swing(): Unit = {
    println("PlasticHandle swing")
    clean()
  }
}

trait Hammer { self : IHandle with IHead =>
  def utilize() : Unit = {
    println()
    println(s"I am a hammer with ${handleName} handle and ${headName} head")
    grab()
  }
}


object Runner {
  object BrassPlasticHammer extends Hammer with PlasticHandle with BrassHead
  object BrassWoodenHammer extends WoodHandle with BrassHead

  object SteelPlasticHammer extends PlasticHandle with SteelHead
  object SteelWoodenHammer extends WoodHandle with SteelHead

  def main( args : Array[String]) : Unit = {
    val instances = List( BrassPlasticHammer, BrassWoodenHammer, SteelPlasticHammer, SteelWoodenHammer)
    instances.foreach( _.sharpen())
  }
}
