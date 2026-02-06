void main() {
  bool a;
  bool b;
  bool r;

  a = true;
  b = false;

  r = a && b;
  r = a || b;
  r = (a || b) && (!b);
}
