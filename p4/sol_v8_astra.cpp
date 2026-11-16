// Astra Masyu solver: CDCL with lazy cycle clauses, constraint-domain local repair,
// and validity-preserving loop shortening. Single-file C++17, standard library only.
// Read R and R grid rows from stdin. Write to OUTPUT_PATH if set, otherwise stdout.
// Fixed RNG seed; fast optimization budget below one second, hard search cap 7.3s.
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <functional>
#include <string>
#include <utility>
#include <vector>
using namespace std;

// Two-watched-literal CDCL engine; all learned clauses remain local to this solve.
class CDCL {
    struct Clause { vector<int> l; };
    function<vector<int>()> theory; int ticks=0;
    int n; vector<Clause> cs; vector<vector<int>> watch;
    vector<int> val,lev,reason,trail,limits; vector<unsigned char> phase,seen;
    vector<double> act; priority_queue<pair<double,int>> heap;
    int head=0; double inc=1; bool okay=true,ready=false;
    chrono::steady_clock::time_point origin;
    double deadline;
    int value(int p)const{return val[p>>1]<0?-1:(val[p>>1]^(p&1));}
    void put(int p,int why){int x=p>>1;val[x]=1-(p&1);phase[x]=val[x];lev[x]=limits.size();reason[x]=why;trail.push_back(p);}
    void bump(int x){act[x]+=inc;heap.emplace(act[x],x);}
    int store(vector<int> a){int id=cs.size();cs.push_back({move(a)});if(cs[id].l.size()>1){watch[cs[id].l[0]].push_back(id);watch[cs[id].l[1]].push_back(id);}return id;}
    int propagate(){
        while(head<(int)trail.size()){
            int bad=trail[head++]^1;auto& list=watch[bad];size_t out=0;
            for(size_t i=0;i<list.size();i++){
                int id=list[i];auto& a=cs[id].l;
                if(a[0]==bad)swap(a[0],a[1]);
                if(value(a[0])==1){list[out++]=id;continue;}
                size_t j=2;while(j<a.size()&&value(a[j])==0)++j;
                if(j<a.size()){swap(a[1],a[j]);watch[a[1]].push_back(id);continue;}
                list[out++]=id;
                if(value(a[0])==0){while(++i<list.size())list[out++]=list[i];list.resize(out);return id;}
                put(a[0],id);
            }
            list.resize(out);
        }
        return -1;
    }
    void cancel(int level){
        if((int)limits.size()<=level)return;
        int end=limits[level];for(int i=trail.size()-1;i>=end;i--){int x=trail[i]>>1;val[x]=-1;heap.emplace(act[x],x);}
        trail.resize(end);head=min(head,end);limits.resize(level);
    }
    pair<vector<int>,int> analyze(int conflict){
        vector<int> result(1,-1),touched;int paths=0,p=-1,index=trail.size()-1;
        do {
            for(int q:cs[conflict].l){int x=q>>1;if(x==(p>>1)||seen[x]||lev[x]==0)continue;
                seen[x]=1;touched.push_back(x);bump(x);
                if(lev[x]==(int)limits.size())++paths;else result.push_back(q);
            }
            while(!seen[trail[index]>>1])--index;
            p=trail[index--];seen[p>>1]=0;--paths;conflict=reason[p>>1];
        }while(paths>0);
        result[0]=p^1;int back=0,pos=1;
        for(int i=1;i<(int)result.size();i++)if(lev[result[i]>>1]>back){back=lev[result[i]>>1];pos=i;}
        if(result.size()>1)swap(result[1],result[pos]);
        for(int x:touched)seen[x]=0;
        return {move(result),back};
    }
public:
    CDCL(int count,chrono::steady_clock::time_point t,double dl):n(count),watch(2*n),val(n,-1),lev(n),reason(n,-1),phase(n,0),seen(n),act(n,0),origin(t),deadline(dl){}
    void add(vector<int> a){
        if(!okay)return;cancel(0);sort(a.begin(),a.end());vector<int>b;
        for(int p:a){if(value(p)==1)return;if(value(p)==0)continue;if(!b.empty()&&b.back()==p)continue;if(!b.empty()&&b.back()==(p^1))return;b.push_back(p);}
        if(b.empty()){okay=false;return;}
        for(int p:b){act[p>>1]+=0.001;if(ready)heap.emplace(act[p>>1],p>>1);}
        int id=store(move(b));if(cs[id].l.size()==1)put(cs[id].l[0],id);
    }
    void initialize(){ready=true;for(int x=0;x<n;x++)heap.emplace(act[x],x);}
    int solve(){
        if(!okay)return 0;
        int conflicts=0,nextRestart=128;
        for(;;){
            if((conflicts&31)==0&&chrono::duration<double>(chrono::steady_clock::now()-origin).count()>=deadline)return -1;
            if(heap.size()>size_t(32*n)){
                priority_queue<pair<double,int>> fresh;
                for(int x=0;x<n;x++)if(val[x]<0)fresh.emplace(act[x],x);
                heap.swap(fresh);
            }
            int c=propagate();
            if(c<0&&theory&&((++ticks&7)==0)){
                auto cut=theory();if(!cut.empty())c=store(move(cut));
            }
            if(c>=0){
                int top=0;for(int p:cs[c].l)top=max(top,lev[p>>1]);cancel(top);
                if(limits.empty()){okay=false;return 0;}
                ++conflicts;auto learned=analyze(c);cancel(learned.second);
                int id=store(move(learned.first));put(cs[id].l[0],id);inc*=1.0526315789473684;
                if(inc>1e90){inc*=1e-90;for(double& a:act)a*=1e-90;heap={};for(int x=0;x<n;x++)if(val[x]<0)heap.emplace(act[x],x);}
                if(conflicts>=nextRestart){cancel(0);nextRestart+=128+(nextRestart/3);}
            }else{
                int x=-1;
                while(!heap.empty()){auto [a,v]=heap.top();heap.pop();if(val[v]<0&&a==act[v]){x=v;break;}}
                if(x<0)return 1;
                limits.push_back(trail.size());put(2*x+(phase[x]?0:1),-1);
            }
        }
    }
    void setTheory(function<vector<int>()> f){theory=move(f);}
    bool selected(int x)const{return val[x]==1;}
    void rootLevel(){cancel(0);}
};

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
    bool satSearch(double end,int margin,bool useTheory,int variant=0){
        vector<array<int,4>> edge(N);for(auto& a:edge)a.fill(-1);
        vector<pair<int,int>> ends;
        for(int x=0;x<N;x++)for(int d=1;d<=2;d++)if(nb[x][d]>=0){int y=nb[x][d],e=ends.size();ends.emplace_back(x,y);edge[x][d]=edge[y][d^2]=e;}
        int E=ends.size();
        if(variant){
            for(int i=E-1;i>0;i--)swap(ends[i],ends[random()%(i+1)]);
            for(int e=0;e<E;e++){auto [x,y]=ends[e];int d=y==x+1?1:2;edge[x][d]=edge[y][d^2]=e;}
        }
        CDCL sat(E+1,start,end);
        sat.add({2*E+1});
        auto lit=[&](int x,int d){return 2*((x>=0&&edge[x][d]>=0)?edge[x][d]:E);};
        for(int x=0;x<N;x++){
            int a[4];for(int d=0;d<4;d++)a[d]=lit(x,d);
            for(int d=0;d<4;d++){vector<int> c={a[d]^1};for(int k=0;k<4;k++)if(k!=d)c.push_back(a[k]);sat.add(c);}
            for(int skip=0;skip<4;skip++){vector<int> c;for(int d=0;d<4;d++)if(d!=skip)c.push_back(a[d]^1);sat.add(c);}
            if(pearl[x]==1){
                sat.add({a[0]^1,a[2]});sat.add({a[2]^1,a[0]});
                sat.add({a[1]^1,a[3]});sat.add({a[3]^1,a[1]});sat.add({a[0],a[1]});
                for(int d=0;d<2;d++)sat.add({a[d]^1,lit(nb[x][d],d)^1,lit(nb[x][d^2],d^2)^1});
            }else if(pearl[x]==2){
                sat.add({a[0],a[2]});sat.add({a[0]^1,a[2]^1});sat.add({a[1],a[3]});sat.add({a[1]^1,a[3]^1});
                for(int d=0;d<4;d++)sat.add({a[d]^1,lit(nb[x][d],d)});
            }
        }
        if(margin>=0){
            int r0=R,c0=C,r1=0,c1=0;for(int x=0;x<N;x++)if(pearl[x]){r0=min(r0,x/C);r1=max(r1,x/C);c0=min(c0,x%C);c1=max(c1,x%C);}
            r0-=margin;r1+=margin;c0-=margin;c1+=margin;
            for(int e=0;e<E;e++){auto [x,y]=ends[e];if(x/C<r0||x/C>r1||x%C<c0||x%C>c1||y/C<r0||y/C>r1||y%C<c0||y%C>c1)sat.add({2*e+1});}
        }
        vector<int> uf(N),usize(N),pcnt(N);
        if(useTheory)sat.setTheory([&](){
            iota(uf.begin(),uf.end(),0);fill(usize.begin(),usize.end(),1);fill(pcnt.begin(),pcnt.end(),0);
            auto find=[&](int x){while(uf[x]!=x){uf[x]=uf[uf[x]];x=uf[x];}return x;};
            vector<int> cycles;
            for(int e=0;e<E;e++)if(sat.selected(e)){
                auto [x,y]=ends[e];int a=find(x),b=find(y);
                if(a==b)cycles.push_back(a);
                else {if(usize[a]<usize[b])swap(a,b);uf[b]=a;usize[a]+=usize[b];}
            }
            if(cycles.empty())return vector<int>{};
            for(int x=0;x<N;x++)if(pearl[x])++pcnt[find(x)];
            for(int v:cycles){int root=find(v);if(pcnt[root]==P)continue;
                vector<int> cut;
                if(pcnt[root]>0){
                    for(int e=0;e<E;e++)if((find(ends[e].first)==root)!=(find(ends[e].second)==root))cut.push_back(2*e);
                }else{
                    for(int e=0;e<E;e++)if(sat.selected(e)&&find(ends[e].first)==root)cut.push_back(2*e+1);
                }
                return cut;
            }
            return vector<int>{};
        });
        sat.initialize();
        while(elapsed()<end){
            int status=sat.solve();if(status!=1)return false;
            vector<int> m(N),component(N,-1);vector<vector<int>> groups;vector<int> pc;
            for(int e=0;e<E;e++)if(sat.selected(e)){
                auto [x,y]=ends[e];int d=y==x+1?1:2;m[x]|=1<<d;m[y]|=1<<(d^2);
            }
            for(int x=0;x<N;x++)if(m[x]&&component[x]<0){
                int id=groups.size(),npearl=0;vector<int> q={x};component[x]=id;
                for(size_t k=0;k<q.size();k++){
                    int v=q[k];npearl+=pearl[v]!=0;
                    for(int d=0;d<4;d++)if(m[v]&(1<<d)){int y=nb[v][d];if(component[y]<0){component[y]=id;q.push_back(y);}}
                }
                groups.push_back(move(q));pc.push_back(npearl);
            }
            for(int id=0;id<(int)groups.size();id++)if(pc[id]==P){
                vector<int> candidate(N,0);for(int x:groups[id])for(int s=1;s<7;s++)if(shape[s]==m[x])candidate[x]=s;
                if(validate(candidate)&&int(groups[id].size())<bestLen){bestLen=groups[id].size();best=move(candidate);return true;}
            }
            sat.rootLevel();
            for(int id=0;id<(int)groups.size();id++){
                vector<int> cut;
                for(int x:groups[id])for(int d=1;d<=2;d++)if(m[x]&(1<<d))cut.push_back(2*edge[x][d]+1);
                sat.add(move(cut));
            }
        }
        return false;
    }
    void shortcutImprove(double end){
        int index[16];fill(index,index+16,-1);for(int s=0;s<7;s++)index[shape[s]]=s;
        while(elapsed()<end){
            vector<int> path;int first=-1;for(int x=0;x<N;x++)if(best[x]){first=x;break;}
            int prev=-1,cur=first;
            do{path.push_back(cur);int next=-1;for(int d=0;d<4;d++)if(shape[best[cur]]&(1<<d)){
                    int y=nb[cur][d];if(y!=prev){next=y;break;}}
                prev=cur;cur=next;
            }while(cur!=first);
            int L=path.size();vector<int> positions;for(int i=0;i<L;i++)if(pearl[path[i]])positions.push_back(i);
            vector<pair<int,int>> gaps;for(int i=0;i<(int)positions.size();i++){
                int j=(i+1)%positions.size(),distance=(positions[j]-positions[i]+L)%L;
                if(distance>6)gaps.emplace_back(-distance,i);
            }
            sort(gaps.begin(),gaps.end());bool improved=false;
            for(auto entry:gaps){
                if(elapsed()>=end)break;
                int i=entry.second,begin=positions[i]+2,finish=positions[i]-entry.first-2;
                int a=path[begin%L],b=path[finish%L],old=finish-begin;
                if(abs(a/C-b/C)+abs(a%C-b%C)>=old)continue;
                vector<unsigned char> blocked(N);for(int x=0;x<N;x++)blocked[x]=best[x]!=0;
                for(int k=begin;k<=finish;k++)blocked[path[k%L]]=0;
                vector<int> pred(N,-1),distance(N,-1),q={a};distance[a]=0;
                for(size_t h=0;h<q.size()&&distance[b]<0;h++){
                    int x=q[h];if(distance[x]+1>=old)continue;
                    for(int y:nb[x])if(y>=0&&!blocked[y]&&distance[y]<0){distance[y]=distance[x]+1;pred[y]=x;q.push_back(y);}
                }
                if(distance[b]<0||distance[b]>=old)continue;
                vector<int> m(N);for(int x=0;x<N;x++)m[x]=shape[best[x]];
                auto toggle=[&](int x,int y){int d=y==x-C?0:y==x+1?1:y==x+C?2:3;m[x]^=1<<d;m[y]^=1<<(d^2);};
                for(int k=begin;k<finish;k++)toggle(path[k%L],path[(k+1)%L]);
                for(int x=b;x!=a;x=pred[x])toggle(x,pred[x]);
                vector<int> candidate(N);bool okay=true;for(int x=0;x<N;x++){candidate[x]=index[m[x]];if(candidate[x]<0)okay=false;}
                if(okay&&validate(candidate)){
                    bestLen+=distance[b]-old;best=move(candidate);improved=true;break;
                }
            }
            if(!improved)break;
        }
    }

    void faceImprove(double end){
        vector<int> current(N);for(int x=0;x<N;x++)current[x]=shape[best[x]];
        int index[16];fill(index,index+16,-1);for(int s=0;s<7;s++)index[shape[s]]=s;
        int length=bestLen,stale=0;
        auto legalPearl=[&](int x){
            if(!pearl[x])return true;int m=current[x];if(!m)return false;
            bool straight=m==5||m==10;if((pearl[x]==1)!=straight)return false;
            int turns=0;for(int d=0;d<4;d++)if(m&(1<<d)){
                int y=nb[x][d];if(y<0)return false;int n=current[y];turns+=n!=5&&n!=10;
            }
            return pearl[x]==1?turns>0:turns==0;
        };
        for(int iter=0;bestLen>lower;iter++){
            if((iter&1023)==0&&(elapsed()>=end||stale>20000+20*N))break;
            ++stale;
            int r=random()%(R-1),c=random()%(C-1),a=r*C+c;
            int cells[4]={a,a+1,a+C+1,a+C};
            int m=0;m|=(current[a]&2)?1:0;m|=(current[a+1]&4)?2:0;
            m|=(current[a+C]&2)?4:0;m|=(current[a]&4)?8:0;
            if(!m||m==15||m==5||m==10)continue;
            int count=__builtin_popcount((unsigned)m),delta=4-2*count;
            if(delta>0&&(random()%1000)>=2)continue;
            int toggles[4]={6,12,9,3},old[4];bool okay=true;
            for(int k=0;k<4;k++){
                old[k]=current[cells[k]];
                int next=old[k]^toggles[k];if(index[next]<0){okay=false;break;}
            }
            if(!okay)continue;
            for(int k=0;k<4;k++)current[cells[k]]^=toggles[k];
            for(int x:cells){
                if(!legalPearl(x))okay=false;
                for(int y:nb[x])if(y>=0&&!legalPearl(y))okay=false;
            }
            if(!okay){for(int k=0;k<4;k++)current[cells[k]]=old[k];continue;}
            length+=delta;
            if(length<bestLen){
                vector<int> candidate(N);for(int x=0;x<N;x++)candidate[x]=index[current[x]];
                if(validate(candidate)){bestLen=length;best=move(candidate);stale=0;}
            }
            if(length>bestLen+6){for(int x=0;x<N;x++)current[x]=shape[best[x]];length=bestLen;}
        }
    }

    void localImprove(double end){
        deadline=end;oneOnly=false;mode=0;nodeLimit=1200;
        int failures=0;
        while(elapsed()<end&&bestLen>lower&&failures<250){
            vector<int> base(N);for(int x=0;x<N;x++)base[x]=1<<best[x];
            int centre=random()%N;for(int tries=0;tries<30&&!best[centre];tries++)centre=random()%N;
            int height=4+random()%9,width=4+random()%9;
            int r0=max(0,centre/C-height/2),r1=min(R,r0+height);
            int c0=max(0,centre%C-width/2),c1=min(C,c0+width);
            for(int r=r0;r<r1;r++)for(int c=c0;c<c1;c++)base[r*C+c]=initial[r*C+c];
            int old=bestLen;reset(base);limit=bestLen;search();
            if(bestLen<old)failures=0;else ++failures;
        }
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
        satSearch(0.30,2,N>500);
        if(best.empty())satSearch(min(7.15,elapsed()+0.7),2,N<=500);
        int attempt=0;
        while(best.empty()&&elapsed()<7.1){
            satSearch(min(7.15,elapsed()+0.6+0.2*attempt),attempt%3==2?-1:2,attempt%2==0,1);
            ++attempt;
        }
        if(!best.empty()&&bestLen>lower){
            if(N<=100){reset(initial);limit=bestLen;oneOnly=false;mode=0;nodeLimit=200000;deadline=0.40;search();}
            double end=min(7.3,max(0.85,elapsed()+0.12));
            faceImprove(min(end,elapsed()+0.12));
            localImprove(max(elapsed(),end-0.08));
            faceImprove(end);
            if(elapsed()<0.88)shortcutImprove(0.90);
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
