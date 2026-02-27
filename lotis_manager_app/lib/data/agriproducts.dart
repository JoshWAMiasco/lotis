abstract class AgriProducts {
  int quantity;
  String name;

  AgriProducts(this.quantity, this.name);
  Future<void> harvest() async {}
  Future<void> sell() async {}
}
