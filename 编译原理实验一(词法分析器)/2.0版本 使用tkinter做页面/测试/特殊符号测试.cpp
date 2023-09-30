#include<iostream>
using namespace std;
int main(){
	int a[] = {1,2,3,4};
	for(int i = 0;i < 4;++i){
		if (a[i] == 5){
			break;
		}
		cout << a[i] << endl;
	}
	return 0;
}