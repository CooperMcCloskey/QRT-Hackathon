int main() {
    ios::sync_with_stdio(false); cin.tie(nullptr);
    if (!(cin >> R) || R < 1) return 0;
    G.resize(R); for (auto& row : G) cin >> row;
    C = G[0].size(); build(); sa::prep(); allPearlsReq();
    const double hardEnd = 7.45;
    double tend = 0.92;
    if (getenv("SEED")) rs ^= (uint64_t)atoll(getenv("SEED")) * 0x9E3779B97F4A7C15ULL;
    auto lenOf = [&](const vector<char>& v) { int l=0; for(char x:v) l+=x; return l; };
    auto validLoop = [&](const vector<char>& v) {
        if ((int)v.size()!=E) return false;
        vector<int> dg(N); int tot=0;
        for(int e=0;e<E;e++) if(v[e]) { dg[eu[e]]++; dg[ev[e]]++; tot++; }
        if(tot<4) return false;
        for(int x=0;x<N;x++) if((dg[x]!=0 && dg[x]!=2)||(isPearl[x] && dg[x]!=2)) return false;
        for(int pi=0;pi<nPearls;pi++) if(!pearlSat(v,pi)) return false;
        return (int)loopOrder(v).size()==tot;
    };
    auto polish = [&](vector<char>& cur,double dur,bool walk) {
        hardPolish(cur,min(hardEnd,elapsed()+dur),0.5,walk);
    };
    vector<char> start(E,0), best;
    for(int c=0;c+1<C;c++) { start[cedge[c*4+1]]=1; start[cedge[((R-1)*C+c)*4+1]]=1; }
    for(int r=0;r+1<R;r++) { start[cedge[(r*C)*4+2]]=1; start[cedge[(r*C+C-1)*4+2]]=1; }
    int bestL=1<<30;
    vector<char> b; int bl=1<<30;
    sa::setState(start);
    sa::run(b,bl,0.25,20.0,3.0,0.3);
    vector<char> hint=sa::on, cand;
    if(!b.empty() && validLoop(b)) cand=b;
    else {
        vector<char> rp=hint;
        if(repairLoop(rp,2.0) && validLoop(rp)) { polish(rp,0.03,false); cand=rp; }
        if(cand.empty() || (N<=1300 && elapsed()<0.45)) {
            vector<char> r;
            if(!cand.empty()) r=satSolve(elapsed()+0.2);
            else {
                double bud=0.35;
                for(int k=0;r.empty() && elapsed()<hardEnd-0.30;k++) {
                    satRandAct=k>=2;
                    r=satSolve(min(hardEnd-0.25,elapsed()+bud),k%2==0?&hint:nullptr);
                    if(k%2==1) bud*=1.8;
                }
                satRandAct=0;
            }
            if(!r.empty() && validLoop(r)) {
                polish(r,0.05,false);
                if(cand.empty() || lenOf(r)<lenOf(cand)) cand=r;
            }
        }
    }
    if(cand.empty() && elapsed()<hardEnd-0.05) {
        satRandAct=1;
        auto r=satSolve(hardEnd-0.02);
        if(!r.empty() && validLoop(r)) cand=r;
    }
    if(!cand.empty()) {
        best=cand; bestL=lenOf(best);
        vector<char> cur=cand;
        tend=min(hardEnd,max(tend,elapsed()+0.25));
        shortcutImprove(cur,min(tend,elapsed()+0.005));
        int fails=0,steps=0;
        while(elapsed()<tend) {
            if(++steps%40==0 && elapsed()+0.012<tend) {
                polish(cur,0.009,true);
                shortcutImprove(cur,min(tend,elapsed()+0.003));
                continue;
            }
            int e0; do { e0=rndi(E); } while(!cur[e0]);
            int x=eu[e0],grow=min(fails/30,6);
            int h=3+grow+rndi(6),w=3+grow+rndi(6);
            int r0=max(0,x/C-rndi(h)),c0=max(0,x%C-rndi(w));
            int r1=min(R-1,r0+h-1),c1=min(C-1,c0+w-1);
            bool ok;
            if(E<600 && rndi(10)==0) ok=lnsStep(cur,0,R-1,0,C-1,2000,tend);
            else ok=lnsStep(cur,r0,r1,c0,c1,500,tend);
            if(ok) fails=0; else fails++;
        }
        if(validLoop(cur) && lenOf(cur)<=bestL) { best=cur; bestL=lenOf(cur); }
    }
    // No bounded solver can promise feasibility on every hidden instance.
    // Retain the original starting loop only when independently valid.
    if(best.empty() && validLoop(start)) { best=start; bestL=lenOf(best); }
    if(best.empty()) best.assign(E,0);
    auto out=render(best);
    const char* path=getenv("OUTPUT_PATH"); FILE* file=path?fopen(path,"w"):stdout;
    if(!file) file=stdout;
    for(auto& row:out) fprintf(file,"%s\n",row.c_str());
    if(file!=stdout) fclose(file);
    cerr<<"bestL="<<bestL<<" t="<<elapsed()<<"\n";
    return 0;
}
