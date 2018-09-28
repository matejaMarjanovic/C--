
int g(int a, double b, int c) {
    int retVal;
    if(a != c) {
        return 18;
    } else {
        return 7;
    }
    return a;
}

int main() {
    double h = 0.0;
    int a;
    int b = 5;
    while(a < 5) {
        b = b+1;
        a = a+1;
    }
    return g(b, h+2.0, a);
}
