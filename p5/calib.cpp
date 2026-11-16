#include <bits/stdc++.h>
using namespace std;
#include WLINC
#define CALIB
#include "core_v3.cpp"
using namespace DL;
int main(int argc,char**argv){
    for(int a=2;a<argc;a++){
        ifstream in(argv[a]); string l; getline(in,l); int n=stoi(l); vector<string> raw(n); for(auto&s:raw) getline(in,s);
        ifstream tr(string(argv[a]).substr(0,strlen(argv[a])-3)+".truth"); getline(tr,l); vector<array<int,3>> T(n); for(auto&t:T) tr>>t[0]>>t[1]>>t[2];
        TLIM=0; g_true=0; solve(raw);
        g_true=1;
        for(int A=0;A<n;A++) for(int B=0;B<n;B++) if(A!=B && T[A][0]>=0 && T[A][0]==T[B][0] && T[B][2]==T[A][2]+pc[A].w){
            int dr=T[B][1]-T[A][1]; if(dr<=-pc[B].h||dr>=pc[A].h) continue; float sc; pairScore(A,B,dr,sc); }
    }
    double cap=atof(getenv("MISSCAP")?getenv("MISSCAP"):"-5");
    FILE*f=fopen(argv[1],"w");
    fprintf(f,"float RH[2][4][10][4]={");
    for(int c=0;c<2;c++){ fprintf(f,"{"); for(int t=0;t<4;t++){ fprintf(f,"{"); for(int l=0;l<10;l++){ fprintf(f,"{");
       double nt=0,nf=0; for(int h=0;h<5;h++){nt+=CN[1][c][t][l][h];nf+=CN[0][c][t][l][h];}
       for(int b=0;b<4;b++){ double pt=(CN[1][c][t][l][b+1]+0.3)/(nt+1.5), pf=(CN[0][c][t][l][b+1]+0.3)/(nf+1.5); double ct=CN[1][c][t][l][b+1]; double v=log(pt/pf); v*= (ct+ (v<0?nt*0.2:0))/(ct+ (v<0?nt*0.2:0) +3.0); v=max(-1.5,min(5.5,v)); fprintf(f,"%.1f%s",v,b<3?",":""); }
       fprintf(f,"}%s",l<9?",":""); } fprintf(f,"}%s",t<3?",":""); } fprintf(f,"}%s",c<1?",":""); } fprintf(f,"};\n");
    fprintf(f,"float RM[2][4][10]={");
    for(int c=0;c<2;c++){ fprintf(f,"{"); for(int t=0;t<4;t++){ fprintf(f,"{"); for(int l=0;l<10;l++){
       double nt=0,nf=0; for(int h=0;h<5;h++){nt+=CN[1][c][t][l][h];nf+=CN[0][c][t][l][h];}
       double pt=(CN[1][c][t][l][0]+0.5)/(nt+1.5), pf=(CN[0][c][t][l][0]+0.5)/(nf+1.5); double v=log(pt/pf); if(nt<8) v*=nt/8.0; v=max(cap,min(0.0,v)); fprintf(f,"%.1f%s",v,l<9?",":"");
       } fprintf(f,"}%s",t<3?",":""); } fprintf(f,"}%s",c<1?",":""); } fprintf(f,"};\n");
    fclose(f);
}
