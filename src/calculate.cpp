#include "catalog.hpp"
#include <algorithm>
#include <cmath>

namespace mathe {
namespace {
struct Out {
 Report r;
 void say(const std::string& s){r.text.push_back(s);}
 void n(const char* s,double x){say(std::string(s)+" = "+number(x));}
 void v(const char* s,Vec x){say(std::string(s)+" = "+vectorText(x));}
 bool need(bool ok,const char* message){if(!ok){r.ok=false;say(std::string("Eingabe nicht geeignet: ")+message);}return ok;}
 void roots(const Roots& r){if(r.all)say("Loesung: alle reellen t.");else if(r.values.empty())say("Keine reelle Loesung.");else for(size_t i=0;i<r.values.size();++i)say("t"+std::to_string(i+1)+" = "+number(r.values[i]));}
};
Vec get(const std::vector<double>& x,int p){return {x[p],x[p+1],x[p+2]};}
bool zero(double value,double scale){return std::abs(value)<=eps*std::max(scale,1e-300);}
bool onPlane(Vec p,Vec n,double d){return zero(dot(n,p)-d,std::abs(n.x*p.x)+std::abs(n.y*p.y)+std::abs(n.z*p.z)+std::abs(d));}
std::string tuple(const std::array<double,4>& a,int n){std::string s="(";for(int i=0;i<n;++i){if(i)s+="; ";s+=number(a[i]);}return s+")";}
void linearReport(Out& out,const Linear& r){
 out.say("Zeilen zuerst durch den groessten Koeffizientenbetrag skalieren (rechte Seite mitteilen).");
 for(const auto& s:r.steps)out.say(s);
 out.say("Reduzierte erweiterte Matrix [A | b]:");
 for(int i=0;i<r.rows;++i){std::string line;for(int j=0;j<r.cols;++j)line+=number(std::abs(r.a[i][j])<=eps?0:r.a[i][j])+"  ";line+="| "+number(r.a[i][r.cols]);out.say(line);}
 out.n("Rang(A)",r.rank);
 if(!r.consistent){out.say("Widerspruch 0 = b mit b ungleich 0. Keine Loesung.");return;}
 out.say("x = "+tuple(r.particular,r.cols));
 for(size_t i=0;i<r.basis.size();++i)out.say("  + t"+std::to_string(i+1)+"*"+tuple(r.basis[i],r.cols));
 if(r.basis.empty())out.say("Eindeutige Loesung.");else out.say("Alle t_i sind frei reell. Unendlich viele Loesungen.");
}
Roots affineZero(Vec a,Vec b){
 int k=0;for(int i=1;i<3;++i)if(std::abs(b[i])>std::abs(b[k]))k=i;
 if(b[k]==0){Roots r;r.all=!valid(a);return r;}
 double t=-a[k]/b[k];Vec p=a+b*t;
 bool iszero=true;for(int i=0;i<3;++i)if(!zero(p[i],std::abs(a[i])+std::abs(b[i]*t)))iszero=false;
 Roots r;if(iszero)r.values.push_back(t);return r;
}
void excludeNull(Out& out,Vec a,Vec b){auto r=affineZero(a,b);if(r.all)out.say("Dieser Vektor ist fuer alle t null: keine Richtung definiert.");else for(double t:r.values)out.say("Ausgeschlossen: t="+number(t)+" (Nullvektor).");}
bool affineNonzero(Vec a,Vec b,double t){Vec p=a+b*t;for(int i=0;i<3;++i)if(!zero(p[i],std::abs(a[i])+std::abs(b[i]*t)))return true;return false;}
}
Report calculate(const Operation& op,const std::vector<double>& x){
 Out o;
 if(!o.need(x.size()==op.fields.size(),"Felder fehlen."))return o.r;
 for(double d:x)if(!o.need(std::isfinite(d)&&std::abs(d)<=1e12&&(d==0||std::abs(d)>=1e-12),"Zahl ausserhalb des Eingabebereichs."))return o.r;
 o.say(op.title);o.say("FORMEL: "+op.formula);
 for(size_t i=0;i<x.size();++i)o.say(op.fields[i].label+" = "+number(x[i]));
 o.say("RECHENWEG");
 const size_t workStart=o.r.text.size();
 switch(op.id){
 case Id::Add:case Id::Sub:{Vec u=get(x,0),v=get(x,3);o.v("u",u);o.v("v",v);for(int i=0;i<3;++i)o.say(std::string(1,"xyz"[i])+": "+number(u[i])+(op.id==Id::Add?" + (":" - (")+number(v[i])+") = "+number(op.id==Id::Add?u[i]+v[i]:u[i]-v[i]));o.v("Ergebnis",op.id==Id::Add?u+v:u-v);break;}
 case Id::Scale:{Vec u=get(x,0);o.say("Jede Komponente mit k="+number(x[3])+" multiplizieren.");o.v("k*u",u*x[3]);break;}
 case Id::Length:case Id::Normalize:{Vec u=get(x,0);double n=norm(u);o.say("|u| = sqrt(("+number(u.x)+")^2+("+number(u.y)+")^2+("+number(u.z)+")^2)");o.n("|u|",n);if(op.id==Id::Normalize&&o.need(valid(u),"Der Nullvektor hat keinen Einheitsvektor.")){o.say("Jede Komponente durch "+number(n)+" teilen.");o.v("e",unit(u));}break;}
 case Id::Dot:case Id::Angle:case Id::Relation:{Vec u=get(x,0),v=get(x,3);o.say("u.v = ("+number(u.x)+")*("+number(v.x)+") + ("+number(u.y)+")*("+number(v.y)+") + ("+number(u.z)+")*("+number(v.z)+")");o.n("u.v",dot(u,v));if(op.id==Id::Dot)break;if(!o.need(valid(u)&&valid(v),"Nullvektoren besitzen keinen Richtungswinkel."))break;o.n("|u|",norm(u));o.n("|v|",norm(v));if(op.id==Id::Relation){o.v("u x v",cross(u,v));o.say(parallel(u,v)?"Die Vektoren sind parallel.":"Die Vektoren sind nicht parallel.");o.say(perpendicular(u,v)?"Die Vektoren sind orthogonal.":"Die Vektoren sind nicht orthogonal.");}o.n("alpha in Grad",angle(u,v));break;}
 case Id::Cross:{Vec u=get(x,0),v=get(x,3);o.say("x: uy*vz-uz*vy = "+number(u.y*v.z)+" - ("+number(u.z*v.y)+")");o.say("y: uz*vx-ux*vz = "+number(u.z*v.x)+" - ("+number(u.x*v.z)+")");o.say("z: ux*vy-uy*vx = "+number(u.x*v.y)+" - ("+number(u.y*v.x)+")");o.v("u x v",cross(u,v));o.n("|u x v|",norm(cross(u,v)));break;}
 case Id::Triple:{Vec u=get(x,0),v=get(x,3),w=get(x,6);o.v("u x v",cross(u,v));o.n("(u x v).w",dot(cross(u,v),w));o.n("Spatvolumen",std::abs(dot(cross(u,v),w)));break;}
 case Id::Displacement:case Id::Midpoint:case Id::Divide:case Id::PointPoint:case Id::LineThrough:{Vec a=get(x,0),b=get(x,3);o.v("B-A",b-a);
  if(op.id==Id::Displacement)break;
  if(op.id==Id::Midpoint){o.v("A+B",a+b);o.v("M=(A+B)/2",(a+b)/2);}
  else if(op.id==Id::Divide){double m=x[6],n=x[7];if(o.need(m+n!=0,"m+n darf nicht null sein.")){o.n("m+n",m+n);o.v("n*A+m*B",a*n+b*m);o.v("T",(a*n+b*m)/(m+n));}}
  else if(op.id==Id::PointPoint)o.n("d=|B-A|",norm(b-a));
  else if(o.need(valid(b-a),"Zwei gleiche Punkte bestimmen keine eindeutige Gerade.")){o.say("g: X = "+vectorText(a)+" + t*"+vectorText(b-a));o.say("t ist beliebig reell.");}
  break;}
 case Id::Collinear:case Id::PlaneThrough:case Id::Triangle:case Id::TrianglePoint:{Vec a=get(x,0),b=get(x,3),c=get(x,6),u=b-a,v=c-a,n=cross(u,v);bool deg=!valid(u)||!valid(v)||parallel(u,v);o.v("u=B-A",u);o.v("v=C-A",v);o.v("n=u x v",n);
  if(op.id==Id::Collinear){o.say(deg?"Die drei Punkte sind kollinear (einschliesslich gleicher Punkte).":"Die drei Punkte sind nicht kollinear.");break;}
  if(!o.need(!deg,"Die Punkte sind kollinear; kein echtes Dreieck / keine eindeutige Ebene."))break;
  if(op.id==Id::PlaneThrough){o.say("E: X = "+vectorText(a)+" + s*"+vectorText(u)+" + t*"+vectorText(v));o.n("d=n.A",dot(n,a));o.say(equation(n,dot(n,a)));o.say("Normalenform: "+vectorText(n)+".(X-"+vectorText(a)+")=0");}
  else if(op.id==Id::Triangle){double f=norm(n)/2;o.n("F=|n|/2",f);o.n("a=|C-B|",norm(c-b));o.n("b=|A-C|",norm(a-c));o.n("c=|B-A|",norm(u));o.n("Umfang",norm(c-b)+norm(v)+norm(u));o.n("Hoehe auf AB: 2F/|AB|",2*f/norm(u));o.n("Winkel A (Grad)",angle(u,v));o.n("Winkel B (Grad)",angle(a-b,c-b));o.n("Winkel C (Grad)",angle(a-c,b-c));o.v("Schwerpunkt",(a+b+c)/3);}
  else {Vec p=get(x,9),w=p-a;std::array<std::array<double,5>,4> m{};for(int i=0;i<3;++i){m[i][0]=u[i];m[i][1]=v[i];m[i][2]=w[i];}auto r=solve(m,3,2);linearReport(o,r);if(!r.consistent){o.say("P liegt ausserhalb der Dreiecksebene, also nicht im Dreieck.");break;}double s=r.particular[0],t=r.particular[1];o.n("s",s);o.n("t",t);o.n("s+t",s+t);bool inside=s>=-eps&&t>=-eps&&s+t<=1+eps;bool boundary=std::abs(s)<=eps||std::abs(t)<=eps||std::abs(s+t-1)<=eps;o.say(!inside?"P liegt ausserhalb des Dreiecks.":boundary?"P liegt auf dem Rand des Dreiecks.":"P liegt im Inneren des Dreiecks.");}break;}
 case Id::Coplanar:case Id::Tetrahedron:{Vec a=get(x,0),u=get(x,3)-a,v=get(x,6)-a,w=get(x,9)-a;Vec n=cross(u,v);double det=dot(n,w);o.v("B-A",u);o.v("C-A",v);o.v("D-A",w);o.v("(B-A) x (C-A)",n);o.n("Determinante",det);if(op.id==Id::Tetrahedron)o.n("V=|det|/6",std::abs(det)/6);else o.say(onPlane(w,n,0)?"Die Punkte sind koplanar.":"Die Punkte sind nicht koplanar.");break;}
 case Id::PlaneParam:{Vec a=get(x,0),u=get(x,3),v=get(x,6);if(!o.need(valid(u)&&valid(v)&&!parallel(u,v),"Spannvektoren muessen linear unabhaengig sein."))break;Vec n=cross(u,v);o.v("n=u x v",n);o.n("d=n.A",dot(n,a));o.say("Normalenform: "+vectorText(n)+".(X-"+vectorText(a)+")=0");o.say(equation(n,dot(n,a)));o.say("Hesse: "+equation(unit(n),dot(n,a)/norm(n)));break;}
 case Id::PlaneCoordinate:{Vec n=get(x,0);double d=x[3];if(!o.need(valid(n),"Normalenvektor darf nicht null sein."))break;int k=0;for(int i=1;i<3;++i)if(std::abs(n[i])>std::abs(n[k]))k=i;Vec axis=k==0?Vec{0,1,0}:Vec{1,0,0};Vec u=unit(cross(unit(n),axis)),v=cross(unit(n),u),a=unit(n)*(d/norm(n));o.v("Einheitsnormale",unit(n));o.n("d/|n|",d/norm(n));o.v("A=(d/|n|)*(n/|n|)",a);o.v("u senkrecht n",u);o.v("v=n0 x u",v);o.say("E: X="+vectorText(a)+" + s*"+vectorText(u)+" + t*"+vectorText(v));o.say("Hesse: "+equation(unit(n),d/norm(n)));break;}
 case Id::PointLine:{Vec p=get(x,0),a=get(x,3),u=get(x,6);if(!o.need(valid(u),"Geradenrichtung darf nicht null sein."))break;Vec w=p-a;double t=dot(w,unit(u))/norm(u);Vec f=a+u*t;o.v("P-A",w);o.n("(P-A).u",dot(w,u));o.n("u.u",dot(u,u));o.n("t",t);o.v("Lotfuss F",f);o.v("P-F",p-f);o.n("Abstand",norm(cross(w,unit(u))));o.v("Spiegelpunkt P'=2F-P",f*2-p);o.say(!valid(w)||parallel(w,u)?"P liegt auf der Geraden.":"P liegt nicht auf der Geraden.");break;}
 case Id::PointPlane:{Vec p=get(x,0),n=get(x,3);double d=x[6];if(!o.need(valid(n),"Normalenvektor darf nicht null sein."))break;double h=(dot(n,p)-d)/norm(n);Vec f=p-unit(n)*h;o.n("n.P-d",dot(n,p)-d);o.n("|n|",norm(n));o.n("Orientierter Abstand h",h);o.n("Abstand |h|",std::abs(h));o.v("Lotfuss F",f);o.v("Spiegelpunkt P'=2F-P",f*2-p);o.say(onPlane(p,n,d)?"P liegt in der Ebene.":"P liegt nicht in der Ebene.");break;}
 case Id::LineLine:{Vec a=get(x,0),u=get(x,3),b=get(x,6),v=get(x,9);if(!o.need(valid(u)&&valid(v),"Beide Geradenrichtungen muessen ungleich null sein."))break;auto r=linePair(a,u,b,v);o.v("u x v",cross(u,v));o.say(r.kind==LinePair::Identical?"Lage: identisch.":r.kind==LinePair::Parallel?"Lage: echt parallel.":r.kind==LinePair::Intersect?"Lage: schneidend.":"Lage: windschief.");o.n("Winkel der Geraden (Grad)",angle(u,v,true));
  if(r.kind==LinePair::Parallel||r.kind==LinePair::Identical)o.say("d = |(B-A) x u|/|u|. Fuer das Lot von B auf g: s=((B-A).u)/(u.u), t=0.");
  else {o.say("Fuer die naechsten Punkte: (A+s*u-B-t*v).u=0 und (A+s*u-B-t*v).v=0.");o.say("Damit: (u.u)*s-(u.v)*t=(B-A).u; (u.v)*s-(v.v)*t=(B-A).v.");o.say("Fuer den Abstand: d=|(B-A).(u x v)|/|u x v|.");}
  o.n("s",r.s);o.n("t",r.t);o.v(r.kind==LinePair::Intersect?"Schnittpunkt S":"Naechster Punkt auf g",r.p);o.v("Zugehoeriger Punkt auf h",r.q);o.n("Abstand",r.distance);break;}
 case Id::LinePlane:case Id::ParamPlane:{Vec a=get(x,0),u=get(x,3),n=get(x,6);double d=x[9];if(!o.need(valid(n),"Normalenvektor darf nicht null sein."))break;if(op.id==Id::LinePlane&&!o.need(valid(u),"Geradenrichtung darf nicht null sein."))break;double nu=dot(n,u),rhs=d-dot(n,a);o.n("n.u bzw. n.w",nu);o.n("d-n.A bzw. d-n.P0",rhs);bool par=!valid(u)||perpendicular(n,u);
  if(par){bool in=onPlane(a,n,d);o.say(in?(op.id==Id::ParamPlane?"Alle reellen t sind Loesungen.":"Die Gerade liegt in der Ebene."):(op.id==Id::ParamPlane?"Keine Loesung.":"Gerade und Ebene sind echt parallel."));o.n("Abstand",std::abs(rhs)/norm(n));}
  else {double t=rhs/nu;o.say(number(nu)+"*t = "+number(rhs));o.n("t",t);o.v("S=A+t*u bzw. P(t)",a+u*t);o.n("Abstand",0);}
  if(op.id==Id::LinePlane)o.n("Winkel Gerade/Ebene (Grad)",90-angle(u,n,true));
  break;}
 case Id::PlanePlane:{Vec n=get(x,0),m=get(x,4);double d=x[3],e=x[7];if(!o.need(valid(n)&&valid(m),"Beide Normalenvektoren muessen ungleich null sein."))break;Vec u=cross(n,m);o.v("u=n1 x n2",u);o.n("Winkel Ebenen (Grad)",angle(n,m,true));
  if(parallel(n,m)){double d1=d/norm(n),d2=(dot(unit(n),unit(m))<0?-1:1)*e/norm(m),h=std::abs(d1-d2);o.n("Orientiert normiertes d1",d1);o.n("Orientiert normiertes d2",d2);o.say(zero(h,std::abs(d1)+std::abs(d2))?"Ebenen sind identisch.":"Ebenen sind echt parallel.");o.n("Abstand",h);}
  else {std::array<std::array<double,5>,4> a{};for(int i=0;i<3;++i){a[0][i]=n[i];a[1][i]=m[i];}a[0][3]=d;a[1][3]=e;auto r=solve(a,2,3,true);linearReport(o,r);if(!r.consistent||r.rank!=2){o.need(false,"Numerisch nahezu parallele Ebenen; Skalierung pruefen.");break;}Vec p{r.particular[0],r.particular[1],r.particular[2]};o.say("Schnittgerade: X="+vectorText(p)+" + t*"+vectorText(u));o.n("Abstand",0);}break;}
 case Id::ProjectVector:{Vec u=get(x,0),v=get(x,3);if(!o.need(valid(v),"Projektionsrichtung darf nicht null sein."))break;double k=dot(u,unit(v))/norm(v);Vec p=v*k;o.n("k=(u.v)/(v.v)",k);o.v("u_parallel",p);o.v("u_senkrecht",u-p);o.n("Kontrolle: Teilvektoren-Skalarprodukt",dot(p,u-p));break;}
 case Id::Parallelogram:{Vec u=get(x,0),v=get(x,3),w=get(x,6),n=cross(u,v);double f=norm(n),vol=std::abs(dot(n,w));o.v("u x v",n);o.n("Grundflaeche F",f);o.n("Spatvolumen V",vol);if(valid(u)&&valid(v)&&!parallel(u,v))o.n("Hoehe V/F",vol/f);else o.say("Entartete Grundflaeche; keine eindeutige Hoehe.");break;}
 case Id::SpherePoint:{Vec m=get(x,0),p=get(x,4);double r=x[3];if(!o.need(r>=0,"Radius muss nichtnegativ sein."))break;double d=norm(p-m);o.v("P-M",p-m);o.n("|P-M|",d);o.n("r",r);o.say(zero(d-r,d+r)?"P liegt auf der Kugeloberflaeche.":d<r?"P liegt innerhalb der Kugel.":"P liegt ausserhalb der Kugel.");o.n("Abstand zur Oberflaeche",std::abs(d-r));o.n("Abstand zum Kugelkoerper",std::max(0.0,d-r));o.n("Kugelvolumen",4*pi*r*r*r/3);o.n("Oberflaeche",4*pi*r*r);break;}
 case Id::SphereLine:{Vec m=get(x,0),a=get(x,4),u=get(x,7);double r=x[3];if(!o.need(r>=0&&valid(u),"Radius >=0 und Geradenrichtung ungleich null erforderlich."))break;Vec w=a-m;double aa=dot(u,u),bb=2*dot(u,w),cc=dot(w,w)-r*r;o.n("a=u.u",aa);o.n("b=2*u.(A-M)",bb);o.n("c=|A-M|^2-r^2",cc);
  // Solve with the closest-point form to avoid cancellation in b^2-4ac.
  double t0=-dot(w,unit(u))/norm(u),h=norm(cross(w,unit(u))),delta=r*r-h*h,scale=r*r+h*h;o.n("t am Lotfuss",t0);o.n("Abstand M zu g",h);Roots roots;
  if(zero(delta,scale))roots.values.push_back(t0);else if(delta>0){double dt=std::sqrt(delta)/norm(u);roots.values={t0-dt,t0+dt};}
  o.say(roots.values.empty()?"Passante: kein Schnittpunkt.":roots.values.size()==1?"Ein Beruehrpunkt (Tangente bei r>0).":"Sekante: zwei Schnittpunkte.");o.roots(roots);for(double t:roots.values)o.v("S=A+t*u",a+u*t);break;}
 case Id::SpherePlane:{Vec m=get(x,0),n=get(x,4);double r=x[3],d=x[7];if(!o.need(r>=0&&valid(n),"Radius >=0 und Normalenvektor ungleich null erforderlich."))break;double h=(dot(n,m)-d)/norm(n),delta=r*r-h*h;Vec f=m-unit(n)*h;o.n("Orientierter Abstand h",h);o.v("Lotfuss / Kreismittelpunkt F",f);o.n("r^2-h^2",delta);if(zero(delta,r*r+h*h)){o.say("Genau ein Beruehrpunkt.");o.v("S",f);}else if(delta<0)o.say("Kein Schnitt.");else {o.n("Schnittkreisradius rho",std::sqrt(delta));o.say("Kreis liegt in der eingegebenen Ebene; Mittelpunkt F.");}break;}
 case Id::SphereSphere:{Vec m=get(x,0),n=get(x,4);double r=x[3],s=x[7];if(!o.need(r>=0&&s>=0,"Radien muessen nichtnegativ sein."))break;Vec w=n-m;double d=norm(w);o.n("D=|M2-M1|",d);
  if(d==0){if(zero(r-s,r+s)){o.say(r==0?"Identische Punktkugeln: genau ein gemeinsamer Punkt.":"Identische Kugeln: gemeinsame gesamte Oberflaeche.");if(r==0)o.v("S",m);}else o.say("Konzentrische Kugeln ohne gemeinsame Oberflaechenpunkte.");break;}
  if((d>r+s&&!zero(d-r-s,d+r+s))||(d<std::abs(r-s)&&!zero(d-std::abs(r-s),d+r+s))){o.say("Kein gemeinsamer Oberflaechenpunkt.");break;}
  double a=(r*r-s*s+d*d)/(2*d);Vec c=m+unit(w)*a;double delta=r*r-a*a;o.n("a",a);o.v("Kreismittelpunkt / Beruehrpunkt",c);if(zero(delta,r*r+a*a)||delta<0)o.say("Die Kugeln beruehren sich in diesem Punkt.");else {o.n("Kreisradius rho",std::sqrt(delta));o.say("Schnittkreisebene: "+equation(w,dot(w,c)));}break;}
 case Id::SphereFour:{Vec a=get(x,0);std::array<std::array<double,5>,4> matrix{};for(int i=0;i<3;++i){Vec b=get(x,3*(i+1)),w=b-a;for(int j=0;j<3;++j)matrix[i][j]=2*w[j];matrix[i][3]=dot(w,b+a);}auto r=solve(matrix,3,3,true);linearReport(o,r);if(r.consistent){Vec m{r.particular[0],r.particular[1],r.particular[2]};o.v(r.basis.empty()?"Mittelpunkt M":"Ein moeglicher Mittelpunkt M0",m);o.n("r=|M-A|",norm(m-a));if(!r.basis.empty())o.say("Jeder Mittelpunkt der angegebenen Loesungsmenge liefert mit r=|M-A| eine passende Kugel.");}break;}
 case Id::ParamLength:{Vec a=get(x,0),b=get(x,3);double l=x[6];if(!o.need(l>=0,"Laenge L muss nichtnegativ sein."))break;double aa=dot(b,b),bb=2*dot(a,b),cc=dot(a,a)-l*l;o.say(number(aa)+"*t^2 + ("+number(bb)+")*t + ("+number(cc)+") = 0");auto r=quadratic(aa,bb,cc);o.roots(r);for(double t:r.values)o.v("u(t)",a+b*t);break;}
 case Id::ParamOrthogonal:case Id::ParamParallel:{Vec a=get(x,0),b=get(x,3),c=get(x,6),d=get(x,9);Roots r;
  if(op.id==Id::ParamOrthogonal){double aa=dot(b,d),bb=dot(a,d)+dot(b,c),cc=dot(a,c);o.say(number(aa)+"*t^2 + ("+number(bb)+")*t + ("+number(cc)+") = 0");r=quadratic(aa,bb,cc);}
  else {Vec aa=cross(b,d),bb=cross(a,d)+cross(b,c),cc=cross(a,c);bool found=false;for(int i=0;i<3;++i){o.say(std::string(1,"xyz"[i])+": "+number(aa[i])+"*t^2+("+number(bb[i])+")*t+("+number(cc[i])+")=0");auto q=quadratic(aa[i],bb[i],cc[i]);if(!found&&!q.all){r=q;found=true;}}if(!found)r.all=true;else {std::vector<double> accepted;for(double t:r.values){bool ok=true;for(int i=0;i<3;++i)if(!zero((aa[i]*t+bb[i])*t+cc[i],std::abs(aa[i]*t*t)+std::abs(bb[i]*t)+std::abs(cc[i])))ok=false;if(ok)accepted.push_back(t);}r.values=accepted;}}
  std::vector<double> accepted;for(double t:r.values)if(affineNonzero(a,b,t)&&affineNonzero(c,d,t))accepted.push_back(t);r.values=accepted;
  if(r.all&&((!valid(a)&&!valid(b))||(!valid(c)&&!valid(d)))){r.all=false;o.say("Ein Vektor ist identisch null; keine zulaessige Richtung.");}
  o.roots(r);if(r.all){excludeNull(o,a,b);excludeNull(o,c,d);}for(double t:r.values){o.v("u(t)",a+b*t);o.v("v(t)",c+d*t);}o.say("Nullvektoren werden fuer diese Richtungsaufgabe ausgeschlossen.");break;}
 case Id::ParamLine:{Vec p=get(x,0),w=get(x,3),a=get(x,6),u=get(x,9);if(!o.need(valid(u),"Die Gerade braucht eine Richtung ungleich null."))break;std::array<std::array<double,5>,4> m{};for(int i=0;i<3;++i){m[i][0]=w[i];m[i][1]=-u[i];m[i][2]=(a-p)[i];}o.say("Unbekanntenreihenfolge: x=(t; s).");auto r=solve(m,3,2,true);linearReport(o,r);if(r.consistent&&r.basis.empty())o.v("P(t)",p+w*r.particular[0]);break;}
 case Id::ParamCoplanar:{Vec a=get(x,0),u=get(x,3)-a,v=get(x,6)-a,d=get(x,9),w=get(x,12),n=cross(u,v);double b=dot(n,w),c=dot(n,d-a);o.v("n=(B-A) x (C-A)",n);o.say(number(b)+"*t + ("+number(c)+") = 0");o.roots(quadratic(0,b,c));break;}
 case Id::Lgs2:case Id::Lgs3:case Id::Lgs4:{int n=op.id==Id::Lgs2?2:op.id==Id::Lgs3?3:4;std::array<std::array<double,5>,4> a{};for(int i=0;i<n;++i)for(int j=0;j<=n;++j)a[i][j]=x[i*(n+1)+j];linearReport(o,solve(a,n,n,true));break;}
 case Id::Combination:case Id::Independence:{Vec u=get(x,0),v=get(x,3),w=get(x,6),p=op.id==Id::Combination?get(x,9):Vec{};std::array<std::array<double,5>,4> a{};for(int i=0;i<3;++i){a[i][0]=u[i];a[i][1]=v[i];a[i][2]=w[i];a[i][3]=p[i];}auto r=solve(a,3,3,true);o.say("Unbekanntenreihenfolge: x=(a; b; c).");linearReport(o,r);if(op.id==Id::Independence){o.n("det(u,v,w)",dot(cross(u,v),w));o.say(r.rank==3?"Linear unabhaengig: Die drei Vektoren bilden eine Basis des R3.":"Linear abhaengig: keine Basis des R3.");if(!r.basis.empty())o.say("Nichttriviale Relation: "+tuple(r.basis[0],3)+" als Koeffizienten von u,v,w ergibt den Nullvektor.");}break;}
 }
 bool overflow=false;
 for(const auto& s:o.r.text)if(s.find("[Zahlenbereich ueberschritten]")!=std::string::npos)overflow=true;
 if(overflow){o.r.ok=false;o.say("Numerische Grenze erreicht; Eingaben skalieren.");}
 // Put the answer before the longer derivation and input list on the small LCD.
 std::vector<std::string> summary;
 auto last=[&](size_t count){size_t start=std::max(workStart,o.r.text.size()-std::min(count,o.r.text.size()));summary.insert(summary.end(),o.r.text.begin()+start,o.r.text.end());};
 auto matching=[&](std::initializer_list<const char*> prefixes){for(size_t i=workStart;i<o.r.text.size();++i)for(const char* p:prefixes)if(o.r.text[i].rfind(p,0)==0){summary.push_back(o.r.text[i]);break;}};
 if(!o.r.ok)last(1);
 else switch(op.id){
  case Id::LineLine:matching({"Lage:","Winkel der Geraden", "Schnittpunkt S", "Abstand ="});break;
  case Id::LinePlane:matching({"Die Gerade", "Gerade und Ebene", "S=A", "Abstand =", "Winkel Gerade"});break;
  case Id::PlanePlane:matching({"Ebenen sind", "Schnittgerade:", "Abstand =", "Winkel Ebenen"});break;
  case Id::PointLine:case Id::PointPlane:matching({"Abstand", "Lotfuss", "Spiegelpunkt", "P liegt"});break;
  case Id::Triangle:matching({"F=", "Umfang", "Winkel A", "Winkel B", "Winkel C"});break;
  case Id::Relation:matching({"Die Vektoren", "alpha"});break;
  case Id::SpherePoint:matching({"P liegt", "Abstand zur", "Abstand zum"});break;
  case Id::SphereLine:matching({"Passante", "Ein Beruehrpunkt", "Sekante", "t1 =", "t2 =", "S=A"});break;
  case Id::ParamOrthogonal:case Id::ParamParallel:case Id::ParamLength:case Id::ParamPlane:case Id::ParamCoplanar:
   matching({"Loesung:", "Alle reellen", "Keine", "t1 =", "t2 =", "t =", "Ausgeschlossen:", "Ein Vektor", "S=A"});break;
  case Id::Lgs2:case Id::Lgs3:case Id::Lgs4:case Id::Combination:case Id::Independence:case Id::ParamLine:
   matching({"x =", "  + t", "Eindeutige", "Alle t_i", "Widerspruch", "Linear unabhaengig", "Linear abhaengig", "Nichttriviale", "P(t) ="});break;
  case Id::PlaneThrough:case Id::PlaneParam:case Id::Parallelogram:case Id::SphereFour:last(3);break;
  case Id::Cross:case Id::Triple:case Id::PlaneCoordinate:case Id::SpherePlane:case Id::SphereSphere:case Id::ProjectVector:last(3);break;
  case Id::LineThrough:last(2);break;
  default:last(1);break;
 }
 if(summary.empty())last(1);
 std::vector<std::string> ordered={op.title,o.r.ok?"ERGEBNIS":"EINGABE PRUEFEN"};
 ordered.insert(ordered.end(),summary.begin(),summary.end());
 ordered.push_back("RECHENWEG");ordered.push_back("FORMEL: "+op.formula);
 ordered.insert(ordered.end(),o.r.text.begin()+workStart,o.r.text.end());
 ordered.push_back("EINGESETZTE WERTE");
 std::string inputs;for(size_t i=0;i<x.size();++i){if(i)inputs+='\n';inputs+=op.fields[i].label+" = "+number(x[i]);}ordered.push_back(inputs);
 o.r.text=std::move(ordered);
 return o.r;
}
}
