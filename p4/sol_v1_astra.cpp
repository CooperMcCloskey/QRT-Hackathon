// Astra: self-contained Masyu constraint search, C++17, standard library only.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
using namespace std;

class Solver {
    using Clock=chrono::steady_clock;
    Clock::time_point start=Clock::now();
    static constexpr int shape[7]={0,10,5,3,9,6,12}; // N,E,S,W
    static constexpr char glyph[8]=".-|LJF7";
    int R,C,N,P=0,bestLen=100000,lower=4;
    vector<string> grid;
    vector<array<int,4>> nb;
    vector<int> pearl,dom,initial,best,deg,mask,parent,sz,req,seen,activity;
    vector<int> queue;
    vector<unsigned char> queued;
    vector<pair<int,int>> trail;
    int edgeSupport[4][128]{},on[4]{},off[4]{},forced[128]{},possible[128]{};
    int forcedCount=0,colourCount[2]{0,0};
    int serial=0,mode=0,limit=100000,nodeLimit=1000000;
    long long nodes=0,totalNodes=0;
    double deadline=0.85;
    bool stopped=false,oneOnly=true;
    unsigned long long rng=0x91a0b16c53ULL;
    unsigned random(){rng^=rng<<7;rng^=rng>>9;return unsigned(rng);}
    double elapsed()const{return chrono::duration<double>(Clock::now()-start).count();}
    bool expired(){if(elapsed()>=deadline){stopped=true;return true;}return false;}
    int colour(int x)const{return ((x/C)+(x%C))&1;}
    void push(int x){if(x>=0&&!queued[x]){queued[x]=1;queue.push_back(x);}}
    bool narrow(int x,int value){
        value &= dom[x];
        if(value==dom[x])return true;
        if(!value){activity[x]+=8;return false;}
        trail.emplace_back(x,dom[x]);
        if((dom[x]&1)&&!(value&1)){++forcedCount;++colourCount[colour(x)];}
        dom[x]=value;push(x);for(int y:nb[x])push(y);
        return true;
    }
    void undo(size_t mark){
        for(int x:queue)queued[x]=0;queue.clear();
        while(trail.size()>mark){auto [x,d]=trail.back();trail.pop_back();
            if((d&1)&&!(dom[x]&1)){--forcedCount;--colourCount[colour(x)];}
            dom[x]=d;
        }
    }
    bool propagate(){
        int ticks=0;
        while(!queue.empty()){
            if((++ticks&1023)==0&&expired())return false;
            int x=queue.back();queue.pop_back();queued[x]=0;
            for(int d=0;d<4;d++){
                int y=nb[x][d];if(y<0)continue;
                int allowed=edgeSupport[d][dom[y]];
                // Black-neighbour rules are binary shape constraints.
                if(pearl[x]==2){
                    int straight=(d&1)?2:4;
                    if(!(dom[y]&straight))allowed&=off[d];
                }
                if(pearl[y]==2){
                    int f=forced[dom[y]];
                    if(f&(1<<(d^2)))allowed&=(d&1)?2:4;
                    // If x cannot be straight along this axis, black y cannot connect.
                    int straight=(d&1)?2:4;
                    if(!(dom[x]&straight)&&!narrow(y,off[d^2]))return false;
                }
                if(!narrow(x,allowed))return false;
            }
            if(pearl[x]==1){
                int viable=0;
                for(int s=1;s<=2;s++)if(dom[x]&(1<<s)){
                    int d=s==1?1:0,a=nb[x][d],b=nb[x][d^2];
                    if(a>=0&&b>=0&&((dom[a]&120)||(dom[b]&120)))viable|=1<<s;
                }
                if(!narrow(x,viable))return false;
                if((dom[x]&(dom[x]-1))==0){
                    int d=dom[x]==2?1:0,a=nb[x][d],b=nb[x][d^2];
                    if(!(dom[a]&120)&&!narrow(b,120))return false;
                    if(!(dom[b]&120)&&!narrow(a,120))return false;
                }
            }
        }
        return true;
    }
    int root(int x){while(parent[x]!=x){parent[x]=parent[parent[x]];x=parent[x];}return x;}
    bool validate(const vector<int>& answer)const{
        int first=-1,count=0;
        for(int x=0;x<N;x++){
            int s=answer[x];if(s<0||s>6)return false;
            if(!s){if(pearl[x])return false;continue;}
            if(first<0)first=x;++count;
            int m=shape[s];
            if(pearl[x]==1&&s>2)return false;
            if(pearl[x]==2&&s<=2)return false;
            int turns=0;
            for(int d=0;d<4;d++)if(m&(1<<d)){
                int y=nb[x][d];if(y<0||!(shape[answer[y]]&(1<<(d^2))))return false;
                turns+=answer[y]>2;
            }
            if(pearl[x]==1&&!turns)return false;
            if(pearl[x]==2&&turns)return false;
        }
        if(first<0)return false;
        int prev=-1,cur=first,walk=0;
        do{int next=-1;for(int d=0;d<4;d++)if(shape[answer[cur]]&(1<<d)){
                int y=nb[cur][d];if(y!=prev){next=y;break;}}
            prev=cur;cur=next;if(cur<0||++walk>count)return false;
        }while(cur!=first);
        return walk==count;
    }
    // Returns -1 contradiction, 0 unfinished, 1 complete valid cycle.
    int connectivity(bool reach){
        iota(parent.begin(),parent.end(),0);fill(sz.begin(),sz.end(),1);
        fill(deg.begin(),deg.end(),0);fill(req.begin(),req.end(),0);
        int cycle=-1;
        for(int x=0;x<N;x++){
            mask[x]=forced[dom[x]];
            for(int d=1;d<=2;d++)if(mask[x]&(1<<d)){
                int y=nb[x][d];if(y<0)return -1;
                ++deg[x];++deg[y];int a=root(x),b=root(y);
                if(a==b){if(cycle>=0)return -1;cycle=x;}
                else {if(sz[a]<sz[b])swap(a,b);parent[b]=a;sz[a]+=sz[b];}
            }
        }
        for(int x=0;x<N;x++)if(!(dom[x]&1))++req[root(x)];
        if(cycle>=0){
            int r=root(cycle);if(req[r]!=forcedCount)return -1;
            vector<int> a(N,0);int len=0;
            for(int x=0;x<N;x++)if(root(x)==r){
                if(__builtin_popcount((unsigned)dom[x])!=1)return -1;
                a[x]=__builtin_ctz((unsigned)dom[x]);++len;
            }
            if(len>=limit||!validate(a))return -1;
            if(len<bestLen){bestLen=len;best=move(a);}
            return 1;
        }
        // A premature closure edge is forbidden even before choosing its shape.
        for(int x=0;x<N;x++)if(deg[x]==1){
            int r=root(x);
            if(req[r]==forcedCount)continue;
            for(int d=0;d<4;d++){
                int y=nb[x][d];
                if(y>x&&deg[y]==1&&root(y)==r&&!(mask[x]&(1<<d))){
                    if(!narrow(x,off[d])||!narrow(y,off[d^2]))return -1;
                }
            }
        }
        if(reach){
            ++serial;vector<int> q;int s=-1;
            for(int x=0;x<N;x++)if(pearl[x]){s=x;break;}
            q.push_back(s);seen[s]=serial;int count=0;
            for(size_t i=0;i<q.size();i++){
                int x=q[i];count+=!(dom[x]&1);
                for(int d=0;d<4;d++)if(possible[dom[x]]&(1<<d)){
                    int y=nb[x][d];if(y>=0&&seen[y]!=serial&&(possible[dom[y]]&(1<<(d^2)))){
                        seen[y]=serial;q.push_back(y);
                    }
                }
            }
            if(count!=forcedCount)return -1;
        }
        return 0;
    }
    bool search(int depth=0){
        if(stopped)return false;
        ++nodes;++totalNodes;
        if(nodes>nodeLimit){stopped=true;return false;}
        if((nodes&63)==0&&expired())return false;
        if(max(forcedCount,2*max(colourCount[0],colourCount[1]))>=limit)return false;
        if(!propagate())return false;
        if(max(forcedCount,2*max(colourCount[0],colourCount[1]))>=limit)return false;
        int status=connectivity(depth<4||(nodes&15)==0);
        if(status<0)return false;
        if(status>0)return true;
        if(!queue.empty()){
            if(!propagate())return false;
            status=connectivity(false);if(status<0)return false;if(status>0)return true;
        }
        int selected=-1;long long high=-(1LL<<60);
        for(int x=0;x<N;x++){
            int k=__builtin_popcount((unsigned)dom[x]);if(k<=1)continue;
            if((dom[x]&1)&&!pearl[x])continue;
            long long score=10000/k+30*min(activity[x],100)+ (pearl[x]?700:0)+(deg[x]==1?1200:0);
            if(mode)score+=random()%600;
            if(score>high){high=score;selected=x;}
        }
        if(selected<0){
            for(int x=0;x<N;x++)if(dom[x]&(dom[x]-1)){selected=x;break;}
        }
        if(selected<0)return false;
        int x=selected;
        vector<pair<int,int>> options;
        for(int s=0;s<7;s++)if(dom[x]&(1<<s)){
            int cost=0;
            for(int d=0;d<4;d++)if(shape[s]&(1<<d)){
                int y=nb[x][d];cost+=(dom[y]&1)?15:0;
                cost-=deg[y]==1?8:0;
                if(best.size()==(size_t)N&&best[x]==s)cost-=5;
            }
            if(mode)cost+=int(random()%20);
            options.emplace_back(cost,s);
        }
        sort(options.begin(),options.end());
        bool found=false;
        for(auto [score,s]:options){
            size_t m=trail.size();
            if(narrow(x,1<<s)&&search(depth+1)){found=true;if(oneOnly){undo(m);return true;}limit=bestLen;}
            undo(m);if(stopped)break;
        }
        if(!found)++activity[x];
        return found;
    }
    void reset(const vector<int>& base){
        dom=base;trail.clear();queue.clear();fill(queued.begin(),queued.end(),0);
        forcedCount=0;colourCount[0]=colourCount[1]=0;
        for(int x=0;x<N;x++){if(!(dom[x]&1)){++forcedCount;++colourCount[colour(x)];}push(x);}
        stopped=false;nodes=0;
    }
public:
    Solver(vector<string> g):R(g.size()),C(g[0].size()),N(R*C),grid(move(g)){
        nb.resize(N);pearl.resize(N);dom.resize(N);initial.resize(N);deg.resize(N);mask.resize(N);
        parent.resize(N);sz.resize(N);req.resize(N);seen.resize(N);activity.resize(N);queued.resize(N);
        for(int d=0;d<4;d++)for(int s=0;s<7;s++)((shape[s]&(1<<d))?on[d]:off[d])|=1<<s;
        for(int m=1;m<128;m++){
            forced[m]=15;for(int s=0;s<7;s++)if(m&(1<<s)){forced[m]&=shape[s];possible[m]|=shape[s];}
            for(int d=0;d<4;d++){
                if(m&on[d^2])edgeSupport[d][m]|=on[d];
                if(m&off[d^2])edgeSupport[d][m]|=off[d];
            }
        }
        int r0=R,r1=0,c0=C,c1=0;
        for(int r=0;r<R;r++)for(int c=0;c<C;c++){
            int x=r*C+c;nb[x]={r?x-C:-1,c+1<C?x+1:-1,r+1<R?x+C:-1,c?x-1:-1};
            pearl[x]=grid[r][c]=='W'?1:grid[r][c]=='B'?2:0;
            initial[x]=pearl[x]==1?6:pearl[x]==2?120:127;
            for(int d=0;d<4;d++)if(nb[x][d]<0)initial[x]&=off[d];
            if(pearl[x]){++P;r0=min(r0,r);r1=max(r1,r);c0=min(c0,c);c1=max(c1,c);}
        }
        lower=max(4,2*(r1-r0+c1-c0));
    }
    vector<string> solve(){
        // Randomized restarts avoid spending the full deadline in one bad ordering.
        int round=0;
        while(best.empty()&&elapsed()<7.2){
            reset(initial);mode=round?1:0;limit=100000;oneOnly=true;
            nodeLimit=min(2000000,2000*(1<<min(round,9)));
            deadline=min(7.3,elapsed()+(round<3?0.18:0.65));
            search();++round;
        }
        if(!best.empty()&&bestLen>lower&&elapsed()<0.8){
            reset(initial);limit=bestLen;oneOnly=false;mode=0;nodeLimit=2000000;deadline=0.85;
            search();
        }
        vector<string> out(R,string(C,'.'));
        if(!best.empty())for(int x=0;x<N;x++)out[x/C][x%C]=glyph[best[x]];
        cerr<<"astra length="<<(best.empty()?0:bestLen)<<" nodes="<<totalNodes<<" time="<<elapsed()<<"\n";
        return out;
    }
};
constexpr int Solver::shape[7];
constexpr char Solver::glyph[8];
int main(){
    ios::sync_with_stdio(false);cin.tie(nullptr);
    int r;if(!(cin>>r)||r<1)return 1;vector<string> g(r);for(auto& row:g)cin>>row;
    Solver solver(move(g));auto result=solver.solve();
    const char* path=getenv("OUTPUT_PATH");FILE* file=path?fopen(path,"w"):stdout;
    if(!file)return 1;for(auto& row:result)fprintf(file,"%s\n",row.c_str());if(path)fclose(file);
}
