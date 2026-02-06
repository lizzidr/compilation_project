void main() {
  int i = 0;
  while (i < 2) {
    i = i + 1;
  }

  for (i = 0; i < 2; i = i + 1) {
    if (i == 1) {
      print("one\n");
    } else {
      print("zero\n");
    }
  }

  do {
    i = i - 1;
  } while (i > 0);
}
