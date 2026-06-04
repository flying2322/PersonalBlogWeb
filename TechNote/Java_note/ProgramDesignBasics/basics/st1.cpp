#include <iostream>
using namespace std;

void Func1(){
    for(int i = 0; i < (1<<6); i++) {
        int A = (i >> 5) & 1;
        int B = (i >> 4) & 1;
        int C = (i >> 3) & 1;
        int D = (i >> 2) & 1;
        int E = (i >> 1) & 1;
        int F = i & 1;
        bool b1 = (A == 1) || (B == 1);
        bool b2 = ((A == 1) && (E == 1)) || ((A == 1) && (F == 1)) || ((E == 1) && (F == 1));
        bool b3 = !((A == 1) && (D == 1));
        bool b4 = ((B == 1) && (C == 1)) || ((B == 0) && (C == 0));
        bool b5 = ((C == 1) && (D == 0)) || ((C == 0) && (D == 1));
        bool b6 = ((D == 0) && (E == 0)) || (D == 1);
        if (b1 && b2 && b3 && b4 && b5 && b6){
            cout << A << B << C << D << E << F <<endl;
            break;
        }
    }
}
int main() {
    cout<<"Hello Naronn"<<endl;
    Func1();
    return 0;
}