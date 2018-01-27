#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <time.h>

using namespace std;

struct HTNode{
	int sym;
	HTNode* leftchild;
	HTNode* rightchild;
	HTNode(int s):leftchild(NULL),rightchild(NULL),sym(s){};
};
struct SymFreq{
	int count;
	HTNode* htnptr;
	SymFreq():count(0),htnptr(NULL){};
};

class HufT{
private:
	unsigned int count[256]; //出現次數為正,offset為symbol
	unsigned int total;//總count
	double average;//編碼平均長度
	unsigned int CodeNumber;//被編碼的數量
	string code[256]; //編碼	
public:
	HufT():total(0),average(0),CodeNumber(256){for(int i=0; i<256; i++)count[i]=0;} //initialize count
	void fetch();//讀取圖片
	void encode();//編碼
	void Post_travel(HTNode*);//遞迴走訪HT
	void print();//輸出到檔案
	HTNode* root;//HT的root
};

void HufT::print(){
	ofstream out("HufT.txt");
	for(int i=0; i<256; i++){
		out<<i<<' '
		   <<(double)count[i]/double(total)<<' '
		   <<code[i]<<endl;
		average+=code[i].length();//累加長度
		if(count[i]==0)CodeNumber--;//如果該symbol的count是0就不會被編碼
	}
	cout<<"average codeword length:"<<(average/=(double)CodeNumber)<<endl;
}

void HufT::Post_travel(HTNode* p){
	static char codeword[256];
	static int level=0;
	if(p->leftchild!=NULL){
		codeword[level++]='0';
		Post_travel(p->leftchild);
	}
	if(p->rightchild!=NULL){
		codeword[level++]='1';
		Post_travel(p->rightchild);
	}
	if(p->leftchild==NULL && p->rightchild==NULL){
		code[p->sym].assign(codeword,level);
	}
	level--;
}

void HufT::fetch(){
	ifstream BMP1("Input_Image1.bmp",ios::binary);
	int symbol=0;
	while((symbol=BMP1.get())!=-1){count[symbol]++;}  //-1是結尾
	BMP1.close();
	ifstream BMP2("Input_Image2.bmp",ios::binary);
	while((symbol=BMP2.get())!=-1){count[symbol]++;}
	BMP2.close();
	
}

void HufT::encode(){
	//建立minheap
	SymFreq sf[257]; //0 isn't used
	int j=0;
	for(int i=0; i<=255; i++){ 
		sf[i+1].count=count[i];
		sf[i+1].htnptr=new HTNode(i);
		j=i+1;
			SymFreq temp=sf[j]; 
			//sf[0].count==0 min
			while(temp.count<sf[j/2].count){
				sf[j]=sf[j/2];	
				j/=2;
			}
			sf[j]=temp;
	}
	
	int last=256;
	//建立Huff tree
	while(last!=1){
		int nosense=256;
		if(sf[1].count==0){//pop but not used
			sf[1]=sf[last];
			sf[last--].count=0;
			SymFreq temp=sf[1];//minheap downheap
			int j=1,k=2;
			while(k <= last && sf[j].count > sf[k].count){ 
				if(k+1 <= last && sf[k].count > sf[k+1].count)k++;
				sf[j]=sf[k];
				j=k;
				k=2*j;
			}
			sf[j]=temp;
		}
		else{//pop and catch
			//get the first node
			int c1=sf[1].count;
			HTNode* p1=sf[1].htnptr;
			sf[1]=sf[last];
			sf[last--].count=0;
			SymFreq temp=sf[1];//minheap downheap
			int j=1,k=2;
			while(k <= last && temp.count > sf[k].count){ 
				if((k+1 <= last )&&( sf[k].count > sf[k+1].count))k++;
				sf[j]=sf[k];
				j=k;
				k=2*j;
			}
			sf[j]=temp;

													//for(int i=1;i<=256;i*=2)cout<<sf[i].count<<endl;
			//get the second node
			int c2=sf[1].count;
			HTNode* p2=sf[1].htnptr;
			sf[1].count=c1+c2;  //the new node will have the two children
			sf[1].htnptr=new HTNode(nosense++); //nosense ++ means every new node is diff

			temp=sf[1];//copy the first
			j=1;k=2;
			while(k <= last && temp.count > sf[k].count){ 
				if((k+1 <= last )&&( sf[k].count > sf[k+1].count))k++;
				sf[j]=sf[k];
				j=k;
				k=2*j;
			}
			sf[j]=temp;
			//將新產生的節點左右子樹設為p1 p2
																	//cout<<c1<<' '<<c2<<endl;
			sf[j].htnptr->leftchild=p1;
			sf[j].htnptr->rightchild=p2;
		}
	}//最後sf[1].htnptr將會是huff tree 的 root
	total=sf[1].count;
	root=sf[1].htnptr;
	Post_travel(sf[1].htnptr);
}

int btod(char b[]){//binary to decimal
	int d=0;
	if(b[0]=='1')d+=128;
	if(b[1]=='1')d+=64;
	if(b[2]=='1')d+=32;
	if(b[3]=='1')d+=16;
	if(b[4]=='1')d+=8;
	if(b[5]=='1')d+=4;
	if(b[6]=='1')d+=2;
	if(b[7]=='1')d+=1;
	return d;
}

class Encoder{
private:
	string code[256];
	char filler[7];
public:
	void get_code();
	void compression();
};

void Encoder::get_code(){
	ifstream huf("HufT.txt");
	for(int i=1,y=0; huf ;i++){
		string discard;
		if(i%3==0)huf>>code[y++];
		else huf>>discard;
	}
	for(int i=0; i<256;i++){
		if(code[i].length()>8){
			for(int j=0;j<7;j++)filler[j]=(code[i])[j];
		break;
		}
	}
}

void Encoder::compression(){
	ifstream BMP1("Input_Image1.bmp",ios::binary);
	int symbol=0,i=0,b=0;
	unsigned long ori_size=0,compre_size=0;//計算壓縮前後的大小
	char buffer[8]={'\0'};
	ofstream huf1("Output_Image1.huf",ios::binary);
	while((symbol=BMP1.get())!=-1){
		ori_size++;
															//cout<<symbol<<endl<<code[symbol]<<endl;system("pause");
		while( i < code[symbol].length() ){
			
			buffer[b++]=(char)(code[symbol])[i++];
			if(b==8){
																	//cout<<btod(buffer)<<endl;
				huf1<<(char)btod(buffer);
				b=0;
				compre_size++;
			}
		}
		i=0;
	}
	if(b!=0){
		while(b!=8)buffer[b++]=filler[i++];
		huf1<<(char)btod(buffer);
		compre_size++;
	}
	cout<<"compression ratio of first one:"
		<<(double)compre_size/(double)ori_size<<endl;

	i=0;b=0;ori_size=0;compre_size=0;//initialize
	BMP1.close();
	huf1.close();
	ifstream BMP2("Input_Image2.bmp",ios::binary);
	ofstream huf2("Output_Image2.huf",ios::binary);
	while((symbol=BMP2.get())!=-1){
		ori_size++;
		while( i < code[symbol].length() ){
			buffer[b++]=(code[symbol])[i++];
			if(b==8){
																	//cout<<(char)btod(buffer);
				huf2<<(char)btod(buffer);
				b=0;
				compre_size++;
			}
		}
		i=0;
	}
	if(b!=0){
		while(b!=8)buffer[b++]=filler[i++];
		huf2<<(char)btod(buffer);
		compre_size++;
	}
	cout<<"compression ratio of second one:"
		<<(double)compre_size/(double)ori_size<<endl;
	BMP2.close();
	huf2.close();
}

class Decoder{
private:
	string code[256];
	HTNode* root;
public:
	Decoder(){root=new HTNode(0);}
	void HTree();
	void decomp();
};

void Decoder::HTree(){
	ifstream huf("HufT.txt");
	for(int i=1,y=0; huf ;i++){
		string discard;
		if(i%3==0)huf>>code[y++];
		else huf>>discard;
	}
	huf.close();

	//plant the Huff tree
	HTNode* p=root;
	for(int i=0; i<256; i++){
		for(int j=0; j<code[i].length(); j++){
			if((code[i])[j]=='0'){
				if(p->leftchild==NULL)p->leftchild=new HTNode(0);
				p=p->leftchild;
			}
			else {
				if(p->rightchild==NULL)p->rightchild=new HTNode(0);
				p=p->rightchild;
			}
		}
		p->sym=i;
		p=root;
	}
}

void Decoder::decomp(){
	ifstream Huf1("Output_Image1.huf",ios::binary);
	ofstream BMP1("Decoded_Image1.bmp",ios::binary);
	HTNode* p=root;
	unsigned char direct,mask=128;
	while(Huf1){
		direct=Huf1.get();
		for(int i=0;i<=7;i++){
			if(p->leftchild==NULL && p->rightchild==NULL){
																//cout<<p->sym;
				BMP1<<(char)p->sym;
				p=root; //go back to root
			}
			if((direct&(mask>>i))==0)p=p->leftchild;
			else p=p->rightchild;
		}
	}
	p=root; //initialize
	Huf1.close();
	BMP1.close();

	ifstream Huf2("Output_Image2.huf",ios::binary);
	ofstream BMP2("Decoded_Image2.bmp",ios::binary);
	while(Huf2){
		direct=Huf2.get();
		for(int i=0;i<=7;i++){
			if(p->leftchild==NULL && p->rightchild==NULL){
																//cout<<p->sym;
				BMP2<<(char)p->sym;
				p=root; //go back to root
			}
			if((direct&(mask>>i))==0)p=p->leftchild;
			else p=p->rightchild;
		}
	}
	Huf2.close();
	BMP2.close();
}
int main(){
	clock_t start,end;
	
	HufT* huftp=new HufT;
	huftp->fetch();
	huftp->encode();
	huftp->print();
		
	Encoder* encp=new Encoder;
	encp->get_code();
	encp->compression();
	//delete encp;

	start = clock();
	Decoder* deco=new Decoder;
	deco->HTree();
	deco->decomp();
	end = clock();
	cout<<"decompression time:"<<(double)(end-start)/(double)CLOCKS_PER_SEC<<endl;

    return 0;

}
