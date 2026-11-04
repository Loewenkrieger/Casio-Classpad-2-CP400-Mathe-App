#include "math.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mathe {
Vec operator+(Vec a,Vec b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
Vec operator-(Vec a,Vec b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec operator*(Vec a,double k){return {a.x*k,a.y*k,a.z*k};}
Vec operator/(Vec a,double k){return a*(1/k);}
double dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
Vec cross(Vec a,Vec b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
double norm(Vec a){return std::hypot(std::hypot(a.x,a.y),a.z);}
bool valid(Vec a){return norm(a)>0;}
Vec unit(Vec a){return a/norm(a);}
bool parallel(Vec a,Vec b){
  if(!valid(a)||!valid(b))return false;
  Vec c=cross(a,b);
  return std::abs(c.x)<=eps*(std::abs(a.y*b.z)+std::abs(a.z*b.y)) &&
         std::abs(c.y)<=eps*(std::abs(a.z*b.x)+std::abs(a.x*b.z)) &&
         std::abs(c.z)<=eps*(std::abs(a.x*b.y)+std::abs(a.y*b.x));
}
bool perpendicular(Vec a,Vec b){return valid(a)&&valid(b)&&std::abs(dot(a,b))<=eps*(std::abs(a.x*b.x)+std::abs(a.y*b.y)+std::abs(a.z*b.z));}
double angle(Vec a,Vec b,bool acute){double c=dot(unit(a),unit(b));return std::acos(std::clamp(acute?std::abs(c):c,-1.0,1.0))*180/pi;}
std::string number(double x){
  if(x==0) return "0";
  if(!std::isfinite(x))return "[Zahlenbereich ueberschritten]";
  const bool negative=x<0;x=std::abs(x);
  int exponent=static_cast<int>(std::floor(std::log10(x)));
  double scaled=x/std::pow(10.0,exponent);
  uint64_t significant=static_cast<uint64_t>(std::floor(scaled*1e9+0.5));
  if(significant>=10000000000ULL){significant/=10;++exponent;}
  std::string digits=std::to_string(significant);
  while(digits.size()<10)digits="0"+digits;
  std::string out;
  if(exponent < -4 || exponent >= 10){
    out=digits.substr(0,1)+"."+digits.substr(1);
    while(out.back()=='0')out.pop_back();
    if(out.back()=='.')out.pop_back();
    out+="e";out+=exponent<0?"-":"+";out+=std::to_string(std::abs(exponent));
  }else {
    int point=exponent+1;
    if(point<=0)out="0."+std::string(-point,'0')+digits;
    else out=digits.substr(0,point)+"."+digits.substr(point);
    while(out.back()=='0')out.pop_back();
    if(out.back()=='.')out.pop_back();
  }
  return (negative?"-":"")+out;
}
std::string vectorText(Vec a){return "("+number(a.x)+"; "+number(a.y)+"; "+number(a.z)+")";}
std::string equation(Vec n,double d){return number(n.x)+"*x + ("+number(n.y)+")*y + ("+number(n.z)+")*z = "+number(d);}

namespace {
class Parser {
  const std::string& s; size_t p=0; int depth=0; bool good=true;
  void space(){while(p<s.size()&&s[p]==' ')++p;}
  bool take(char c){space();if(p<s.size()&&s[p]==c){++p;return true;}return false;}
  bool word(const char* w){space();size_t q=p;while(*w){if(q>=s.size()||s[q++]!=*w++)return false;}p=q;return true;}
  double atom(){
    if(++depth>24){good=false;--depth;return 0;}
    double x=0;
    if(take('(')){x=sum();if(!take(')'))good=false;}
    else if(word("sqrt")){if(!take('('))good=false;x=sum();if(!take(')')||x<0)good=false;else x=std::sqrt(x);}
    else if(word("pi"))x=pi;
    else {
      space();bool digit=false;
      while(p<s.size()&&s[p]>='0'&&s[p]<='9'){digit=true;x=x*10+s[p++]-'0';}
      if(p<s.size()&&s[p]=='.'){++p;double factor=.1;while(p<s.size()&&s[p]>='0'&&s[p]<='9'){digit=true;x+=(s[p++]-'0')*factor;factor*=.1;}}
      if(!digit)good=false;
      if(p<s.size()&&(s[p]=='e'||s[p]=='E')){++p;bool minus=false;if(p<s.size()&&(s[p]=='+'||s[p]=='-'))minus=s[p++]=='-';int e=0;bool edigit=false;while(p<s.size()&&s[p]>='0'&&s[p]<='9'){edigit=true;e=std::min(999,e*10+s[p++]-'0');}if(!edigit||e>308)good=false;x*=std::pow(10.0,minus?-e:e);}
    }
    --depth;return x;
  }
  double power(){double x=atom();if(take('^')){double base=x;x=std::pow(x,unary());if(base!=0&&x==0)good=false;}return x;}
  double unary(){if(take('+'))return unaryGuard(1);if(take('-'))return unaryGuard(-1);return power();}
  double unaryGuard(double sign){if(++depth>24){good=false;--depth;return 0;}double x=sign*unary();--depth;return x;}
  double product(){double x=unary();while(good){if(take('*')){double old=x,y=unary();x*=y;if(old!=0&&y!=0&&x==0)good=false;}else if(take('/')){double y=unary();if(y==0)good=false;else {double old=x;x/=y;if(old!=0&&x==0)good=false;}}else break;}return x;}
  double sum(){double x=product();while(good){if(take('+'))x+=product();else if(take('-'))x-=product();else break;}return x;}
public:
  explicit Parser(const std::string& text):s(text){}
  Parsed run(){double x=sum();space();if(p!=s.size()||!good||!std::isfinite(x))return {false,0,"Ungueltiger Ausdruck / Definitionsbereich."};if(std::abs(x)>1e12||(x!=0&&std::abs(x)<1e-12))return {false,0,"Wert: 0 oder Betrag 1e-12 bis 1e12."};return {true,x,{}};}
};
}
Parsed parse(const std::string& text){std::string s=text;for(char& c:s)if(c==',')c='.';if(s.size()>96)return {false,0,"Ausdruck zu lang."};return Parser(s).run();}

Linear solve(std::array<std::array<double,5>,4> a,int rows,int cols,bool trace){
  Linear r;r.rows=rows;r.cols=cols;
  if(rows<1||rows>4||cols<1||cols>4){r.consistent=false;return r;}
  std::array<double,4> rhsBound{};
  for(int i=0;i<rows;++i){
    double scale=0;for(int j=0;j<cols;++j)scale=std::max(scale,std::abs(a[i][j]));
    if(scale==0){if(a[i][cols]!=0)r.consistent=false;}
    else for(int j=0;j<=cols;++j)a[i][j]/=scale;
    rhsBound[i]=std::abs(a[i][cols]);
  }
  for(int col=0;col<cols&&r.rank<rows;++col){
    int k=r.rank;for(int i=k+1;i<rows;++i)if(std::abs(a[i][col])>std::abs(a[k][col]))k=i;
    if(std::abs(a[k][col])<=eps)continue;
    if(k!=r.rank){std::swap(a[k],a[r.rank]);std::swap(rhsBound[k],rhsBound[r.rank]);if(trace)r.steps.push_back("Z"+std::to_string(k+1)+" <-> Z"+std::to_string(r.rank+1));}
    k=r.rank;double pivot=a[k][col];
    if(trace)r.steps.push_back("Z"+std::to_string(k+1)+" / ("+number(pivot)+")");
    for(int j=0;j<=cols;++j)a[k][j]/=pivot;
    rhsBound[k]/=std::abs(pivot);
    for(int i=0;i<rows;++i)if(i!=k){double factor=a[i][col];if(trace&&std::abs(factor)>eps)r.steps.push_back("Z"+std::to_string(i+1)+" - ("+number(factor)+")*Z"+std::to_string(k+1));for(int j=0;j<=cols;++j)a[i][j]-=factor*a[k][j];rhsBound[i]+=std::abs(factor)*rhsBound[k];}
    r.pivot[r.rank++]=col;
  }
  for(int i=r.rank;i<rows;++i)if(std::abs(a[i][cols])>eps*std::max(rhsBound[i],1e-300))r.consistent=false;
  r.a=a;
  if(!r.consistent)return r;
  for(int i=0;i<r.rank;++i)r.particular[r.pivot[i]]=a[i][cols];
  for(int j=0;j<cols;++j){bool free=true;for(int i=0;i<r.rank;++i)if(r.pivot[i]==j)free=false;if(!free)continue;std::array<double,4> b{};b[j]=1;for(int i=0;i<r.rank;++i)b[r.pivot[i]]=-a[i][j];r.basis.push_back(b);}
  return r;
}
Roots quadratic(double a,double b,double c){
  Roots r;double scale=std::max({std::abs(a),std::abs(b),std::abs(c)});
  if(scale==0){r.all=true;return r;}a/=scale;b/=scale;c/=scale;
  if(a==0){if(b!=0)r.values.push_back(-c/b);return r;}
  double d=b*b-4*a*c, tol=eps*(b*b+std::abs(4*a*c));
  if(d < -tol)return r;
  if(std::abs(d)<=tol){r.values.push_back(-b/(2*a));return r;}
  double q=-0.5*(b+std::copysign(std::sqrt(d),b));
  r.values={q/a,c/q};std::sort(r.values.begin(),r.values.end());return r;
}
LinePair linePair(Vec a,Vec u,Vec b,Vec v){
  LinePair r;Vec un=unit(u),vn=unit(v),w=b-a,k=cross(un,vn);double k2=dot(k,k);
  if(parallel(u,v)){r.s=dot(w,un)/norm(u);r.t=0;r.p=a+u*r.s;r.q=b;r.distance=norm(cross(w,un));r.kind=!valid(w)||parallel(w,u)?LinePair::Identical:LinePair::Parallel;return r;}
  r.s=dot(cross(w,vn),k)/k2/norm(u);
  r.t=dot(cross(w,un),k)/k2/norm(v);
  r.p=a+u*r.s;r.q=b+v*r.t;
  r.distance=std::abs(dot(w,unit(k)));
  r.kind=!valid(w)||perpendicular(w,k)?LinePair::Intersect:LinePair::Skew;
  return r;
}
}
