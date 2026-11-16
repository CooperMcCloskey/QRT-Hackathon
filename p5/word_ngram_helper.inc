static vector<float> ngCount, ngPenalty;
static vector<unsigned char> ngQ;
static const int ngBase[6]={0,0,27,756,20439,551880};
static void ngStart(){ngCount.assign(14900787,0);}
static void ngWord(const string& word,float weight){
    string s=" "+word+" ";
    for(int i=0;i<(int)s.size();i++){
        int code=0;
        for(int n=1;n<=5 && i+n<=(int)s.size();n++){
            int c=s[i+n-1]==' '?26:s[i+n-1]-'a';
            code=code*27+c;ngCount[ngBase[n]+code]+=weight;
        }
    }
}
static void ngFinish(){
    ngQ.assign(14900787,255);ngPenalty.assign(551880,0);
    double total=0;for(int c=0;c<27;c++)total+=ngCount[c];
    for(int c=0;c<27;c++)ngQ[c]=min(254,(int)round(-16*log((ngCount[c]+1)/(total+27))));
    for(int n=2,power=729,low=27;n<=5;n++,power*=27,low*=27){
        for(int k=0;k<power;k++){
            float count=ngCount[ngBase[n]+k];
            int prefix=ngBase[n-1]+k/27;
            if(k%27==0)ngPenalty[prefix]=log(8.0/(ngCount[prefix]+8));
            if(count==0)continue;
            int suffix=k%low;float lp=0;
            for(int m=1,mod=27;m<n;m++,mod*=27){
                int key=suffix%mod;unsigned q=ngQ[ngBase[m]+key];
                if(q!=255)lp=-q*(1.0f/16);
                else lp+=ngPenalty[ngBase[m-1]+key/27];
            }
            double p=(count+8*exp(lp))/(ngCount[prefix]+8);
            ngQ[ngBase[n]+k]=min(254,(int)round(-16*log(max(p,1e-10))));
        }
    }
    vector<float>().swap(ngCount);
}
static inline float ngProb(const char* row,int at,int lo){
    int code=0,mul=1;float value=-3.5f;
    for(int n=1;n<=5 && at-n+1>=lo;n++){
        char ch=row[at-n+1];if(!ch)break;
        int c=cls(ch);if(c>26)c=26;
        code+=c*mul;mul*=27;
        unsigned q=ngQ[ngBase[n]+code];
        if(q!=255)value=-q*(1.0f/16);
        else if(n>1)value+=ngPenalty[ngBase[n-1]+code/27];
    }
    return max(-16.0f,value);
}
static inline float ngSeam(const char* row,int seam,int end){
    float gain=0;
    for(int p=seam;p<min(seam+4,end)&&row[p];p++)gain+=ngProb(row,p,max(0,p-4))-ngProb(row,p,seam);
    return gain;
}
