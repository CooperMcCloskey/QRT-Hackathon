// Quantized character model trained exclusively on public WikiText-2 TRAIN.
// No puzzle input, reference placement or validation text is embedded.
static vector<unsigned char> ngQ;
static const int ngBase[6]={0,0,27,756,20439,551880};
static void initNgram() {
    ngQ.assign(14900787,255);
    vector<unsigned char> bytes; unsigned acc=0; int bits=0;
    for(const char* s=NGRAM_DATA;*s;s++) {
        if(*s=='=') break;
        acc=(acc<<6)|dec64(*s); bits+=6;
        if(bits>=8) { bits-=8;bytes.push_back((acc>>bits)&255); }
    }
    unsigned key=0; size_t i=0;
    while(i<bytes.size()) {
        unsigned delta=0;int shift=0;
        do { unsigned v=bytes[i++];delta|=(v&127)<<shift;shift+=7;if(!(v&128))break; }while(i<bytes.size());
        if(i>=bytes.size())break;
        key+=delta;unsigned q=bytes[i++];
        if(key<ngQ.size())ngQ[key]=min(q,254u);
    }
}
static inline float ngProb(const char* row,int at,int lo) {
    int code=0,mul=1;
    float value=-3.5f;
    for(int n=1;n<=5 && at-n+1>=lo;n++) {
        char ch=row[at-n+1];if(!ch)break;
        int v=cls(ch);if(v>26)v=26;
        code+=v*mul;mul*=27;
        unsigned char q=ngQ[ngBase[n]+code];
        if(q!=255)value=-q*(1.0f/16);
    }
    return value;
}
static inline float ngSeam(const char* row,int seam,int end) {
    float gain=0;
    for(int p=seam;p<min(seam+4,end)&&row[p];p++)
        gain+=ngProb(row,p,max(0,p-4))-ngProb(row,p,seam);
    return gain;
}
