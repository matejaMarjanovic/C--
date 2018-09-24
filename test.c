int f(int a, double b, int c, double d);

int g(int a1, double b1, int c1, double d1) {
    a1 = c1 + d1;
    b1 = c1 - d1;
    return a1 + b1;
}

int main() {
    double a = 5.0;
    int c = 4;
    int b = c+17;
    return b;
}
