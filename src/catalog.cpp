#include "catalog.hpp"
#include <algorithm>
#include <cmath>

namespace mathe {
const std::vector<std::string>& categories(){
 static const std::vector<std::string> c={"Vektoren","Punkte & Teilverhaeltnisse","Geraden & Ebenen aufstellen","Lage, Schnitt & Abstand","Projektion, Flaeche, Volumen","Kugeln","Parameteraufgaben","LGS & Linearkombinationen"};return c;
}
const std::vector<Operation>& operations(){
 static const auto ops=[](){
  std::vector<Operation> a;
  auto add=[&](Id id,int cat,const char* title,const char* formula,const char* method,bool planar=false)->Operation&{a.push_back({id,cat,title,formula,method,{},planar});return a.back();};
  auto v=[](Operation& o,const char* name,Vec initial){for(int i=0;i<3;++i)o.fields.push_back({std::string(name)+"."+"xyz"[i],initial[i]});};
  auto k=[](Operation& o,const char* name,double initial){o.fields.push_back({name,initial});};
  auto two=[&](Operation& o){v(o,"u",{1,2,3});v(o,"v",{2,-1,1});};
  auto line=[&](Operation& o,const char* p,const char* d,Vec pv,Vec dv){v(o,p,pv);v(o,d,dv);};
  auto plane=[&](Operation& o,const char* n,const char* d,Vec nv,double dv){v(o,n,nv);k(o,d,dv);};
  for(Id id:{Id::Add,Id::Sub,Id::Dot,Id::Cross,Id::Angle,Id::Relation}){
   const char *title="",*formula="",*method="";
   switch(id){
    case Id::Add:title="Vektoren addieren";formula="u+v = (ux+vx; uy+vy; uz+vz)";method="Addiere die einander entsprechenden Komponenten.";break;
    case Id::Sub:title="Vektoren subtrahieren";formula="u-v = (ux-vx; uy-vy; uz-vz)";method="Subtrahiere komponentenweise. Die Reihenfolge ist wichtig.";break;
    case Id::Dot:title="Skalarprodukt";formula="u.v = ux*vx + uy*vy + uz*vz";method="Multipliziere entsprechende Komponenten und addiere die Produkte. Das Ergebnis ist eine Zahl.";break;
    case Id::Cross:title="Kreuzprodukt";formula="u x v = (uy*vz-uz*vy; uz*vx-ux*vz; ux*vy-uy*vx)";method="Das Kreuzprodukt steht senkrecht auf beiden Vektoren. Sein Betrag ist die Parallelogrammflaeche. In 2D liefert die z-Komponente die orientierte Flaeche.";break;
    case Id::Angle:title="Winkel zwischen Vektoren";formula="cos(alpha) = (u.v)/(|u|*|v|)";method="Der Vektorwinkel liegt zwischen 0 und 180 Grad. Bei einem Nullvektor ist kein Winkel definiert.";break;
    default:title="Parallel / orthogonal?";formula="parallel: u x v=0; orthogonal: u.v=0";method="Fuer Richtungen muessen beide Vektoren ungleich null sein. Das Skalarprodukt mit dem Nullvektor ist zwar null, aber eine Richtung ist nicht definiert.";break;
   }
   two(add(id,0,title,formula,method,true));
  }
  {auto& o=add(Id::Scale,0,"Mit Skalar multiplizieren","k*u = (k*ux; k*uy; k*uz)","Ein negatives k kehrt die Orientierung um; |k| skaliert die Laenge.",true);v(o,"u",{1,2,3});k(o,"k",2);}
  for(Id id:{Id::Length,Id::Normalize}){auto& o=add(id,0,id==Id::Length?"Betrag / Laenge":"Einheitsvektor",id==Id::Length?"|u| = sqrt(ux^2+uy^2+uz^2)":"e = u/|u|","Der Betrag ist nie negativ. Ein Einheitsvektor hat die Laenge 1. Der Nullvektor kann nicht normiert werden.",true);v(o,"u",{3,4,0});}
  {auto& o=add(Id::Triple,0,"Spatprodukt / Determinante","det(u,v,w) = (u x v).w","Das Vorzeichen beschreibt die Orientierung, der Betrag das Spatvolumen. Null bedeutet lineare Abhaengigkeit.");two(o);v(o,"w",{0,1,2});}
  for(Id id:{Id::Displacement,Id::Midpoint,Id::Divide,Id::PointPoint,Id::LineThrough}){
   const char* title=id==Id::Displacement?"Verbindungsvektor AB":id==Id::Midpoint?"Mittelpunkt":id==Id::Divide?"Teilpunkt (gerichtetes Verhaeltnis)":id==Id::PointPoint?"Punkt - Punkt":"Gerade durch zwei Punkte";
   const char* f=id==Id::Displacement?"AB = B-A":id==Id::Midpoint?"M = (A+B)/2":id==Id::Divide?"AT:TB = m:n; T=(n*A+m*B)/(m+n)":id==Id::PointPoint?"d(A,B) = |B-A|":"g: X=A+t*(B-A)";
   auto& o=add(id,id==Id::PointPoint?3:id==Id::LineThrough?2:1,title,f,"A und B sind Ortsvektoren. Beim Teilpunkt werden gerichtete Verhaeltnisse benutzt: negative Werte erlauben aeussere Teilung; m+n darf nicht null sein.",id!=Id::LineThrough);
   v(o,"A",{1,0,0});v(o,"B",{3,2,0});if(id==Id::Divide){k(o,"m",1);k(o,"n",2);}
  }
  for(Id id:{Id::Collinear,Id::PlaneThrough,Id::Triangle,Id::TrianglePoint}){
   const char* title=id==Id::Collinear?"Drei Punkte auf einer Geraden?":id==Id::PlaneThrough?"Ebene durch drei Punkte":id==Id::Triangle?"Dreieck: Flaeche & Winkel":"Punkt im Dreieck?";
   const char* f=id==Id::TrianglePoint?"P=A+s*(B-A)+t*(C-A); s,t>=0, s+t<=1":id==Id::Triangle?"F = |(B-A) x (C-A)|/2":"n = (B-A) x (C-A)";
   auto& o=add(id,id==Id::Collinear||id==Id::TrianglePoint?1:id==Id::PlaneThrough?2:4,title,f,"Sind B-A und C-A parallel, bestimmen die Punkte keine eindeutige Ebene und kein echtes Dreieck. Die Punktprobe im Dreieck prueft auch die Zugehoerigkeit zur Dreiecksebene.",id!=Id::PlaneThrough);
   v(o,"A",{0,0,0});v(o,"B",{4,0,0});v(o,"C",{0,3,0});if(id==Id::TrianglePoint)v(o,"P",{1,1,0});
  }
  for(Id id:{Id::Coplanar,Id::Tetrahedron}){auto& o=add(id,id==Id::Coplanar?1:4,id==Id::Coplanar?"Vier Punkte in einer Ebene?":"Tetraeder / Pyramidenvolumen",id==Id::Coplanar?"det(B-A,C-A,D-A)=0":"V = |det(B-A,C-A,D-A)|/6","Das Spatprodukt misst das orientierte Spatvolumen. Bei null liegen die vier Punkte in mindestens einer gemeinsamen Ebene.");v(o,"A",{});v(o,"B",{1,0,0});v(o,"C",{0,2,0});v(o,"D",{0,0,3});}
  {auto& o=add(Id::PlaneParam,2,"Parameter- in Koordinatenform","E: X=A+s*u+t*v; n=u x v; d=n.A","Die Spannvektoren muessen linear unabhaengig sein. Normalenform: n.(X-A)=0; Koordinatenform: n.X=d.");v(o,"A",{1,0,0});two(o);}
  {auto& o=add(Id::PlaneCoordinate,2,"Koordinaten- in Parameterform","E: n.X=d; Hesse: (n/|n|).X=d/|n|","Ein Stuetzpunkt erfuellt n.A=d. Zwei unabhaengige Vektoren senkrecht zu n spannen die Ebene auf. Die Hessesche Normalenform nutzt einen Einheitsnormalenvektor.");plane(o,"n","d",{1,2,3},6);}
  {auto& o=add(Id::PointLine,3,"Punkt - Gerade","t=((P-A).u)/(u.u); F=A+t*u; d=|P-F|","g: X=A+t*u. Der Lotfuss F liegt auf g, P-F steht senkrecht zu u. Spiegelpunkt: P'=2*F-P.");v(o,"P",{1,2,3});line(o,"A","u",{}, {1,0,0});}
  {auto& o=add(Id::PointPlane,3,"Punkt - Ebene","h=(n.P-d)/|n|; F=P-h*n/|n|","E: n.X=d. |h| ist der Abstand. Das Vorzeichen von h haengt von der Orientierung von n ab. Spiegelpunkt: P'=2*F-P.");v(o,"P",{1,2,3});plane(o,"n","d",{0,0,1},1);}
  {auto& o=add(Id::LineLine,3,"Gerade - Gerade","A+s*u = B+t*v","g: X=A+s*u; h: X=B+t*v. Parallele Richtungen: Stuetzpunktprobe entscheidet zwischen identisch und echt parallel. Sonst gibt es einen Schnittpunkt oder die Geraden sind windschief. Der Schnittwinkel ist der kleinere Winkel (0 bis 90 Grad).");line(o,"A","u",{}, {1,0,0});line(o,"B","v",{0,0,1},{0,1,0});}
  {auto& o=add(Id::LinePlane,3,"Gerade - Ebene","t=(d-n.A)/(n.u)","g: X=A+t*u; E: n.X=d. Bei n.u=0 liegt g in E oder ist parallel. Sonst gibt es genau einen Schnittpunkt. sin(alpha)=|n.u|/(|n|*|u|).");line(o,"A","u",{1,2,3},{1,0,-1});plane(o,"n","d",{0,0,1},0);}
  {auto& o=add(Id::PlanePlane,3,"Ebene - Ebene","Richtung der Schnittgeraden: u=n1 x n2","E1: n1.X=d1; E2: n2.X=d2. Bei parallelen Normalen sind die Ebenen identisch oder echt parallel. Sonst ergibt ein LGS einen Stuetzpunkt der Schnittgeraden.");plane(o,"n1","d1",{0,0,1},0);plane(o,"n2","d2",{0,1,0},2);}
  {auto& o=add(Id::ProjectVector,4,"Vektorprojektion / Zerlegung","u_parallel = ((u.v)/(v.v))*v; u_senkrecht=u-u_parallel","Die Projektionsrichtung v darf nicht null sein. Beide Teilvektoren ergeben zusammen u und stehen senkrecht aufeinander.",true);two(o);}
  {auto& o=add(Id::Parallelogram,4,"Parallelogramm / Spat","F=|u x v|; V=|(u x v).w|","u und v spannen die Grundflaeche auf; w bildet die dritte Spatkante. Die Hoehe ist V/F.");two(o);v(o,"w",{0,0,2});}
  {auto& o=add(Id::SpherePoint,5,"Kugel: Punktprobe / Abstand","K: |X-M|^2=r^2","M ist der Mittelpunkt, r>=0 der Radius. Vergleiche |P-M| mit r: innen, auf der Oberflaeche oder aussen. Abstand zur Oberflaeche: ||P-M|-r|.");v(o,"M",{});k(o,"r",2);v(o,"P",{3,0,0});}
  {auto& o=add(Id::SphereLine,5,"Kugel - Gerade","|A+t*u-M|^2=r^2","Einsetzen der Geraden ergibt eine quadratische Gleichung: a=u.u, b=2*u.(A-M), c=|A-M|^2-r^2. Keine, eine oder zwei reelle Loesungen entsprechen Passante, Tangente oder Sekante.");v(o,"M",{});k(o,"r",2);line(o,"A","u",{-3,0,0},{1,0,0});}
  {auto& o=add(Id::SpherePlane,5,"Kugel - Ebene","h=(n.M-d)/|n|; rho=sqrt(r^2-h^2)","Der Lotfuss des Kugelmittelpunkts auf der Ebene ist der Mittelpunkt des Schnittkreises. Abstand groesser als r: kein Schnitt; gleich r: Beruehrpunkt.");v(o,"M",{});k(o,"r",2);plane(o,"n","d",{0,0,1},1);}
  {auto& o=add(Id::SphereSphere,5,"Kugel - Kugel","a=(r1^2-r2^2+D^2)/(2*D)","D=|M2-M1|. Bei einem Schnittkreis liegt sein Mittelpunkt a Laengeneinheiten von M1 in Richtung M2. rho=sqrt(r1^2-a^2). Konzentrische und identische Kugeln werden getrennt behandelt.");v(o,"M1",{});k(o,"r1",2);v(o,"M2",{3,0,0});k(o,"r2",2);}
  {auto& o=add(Id::SphereFour,5,"Kugel durch vier Punkte","2*(B-A).M=|B|^2-|A|^2 (analog C,D)","Durch Subtraktion der Kugelgleichungen faellt |M|^2 weg. Loese das entstehende LGS nach M, dann r=|M-A|. Abhaengige Bedingungen liefern keine oder unendlich viele Kugeln.");v(o,"A",{1,0,0});v(o,"B",{0,1,0});v(o,"C",{0,0,1});v(o,"D",{-1,0,0});}
  for(Id id:{Id::ParamOrthogonal,Id::ParamParallel}){auto& o=add(id,6,id==Id::ParamOrthogonal?"Parameter: u(t) senkrecht v(t)":"Parameter: u(t) parallel v(t)",id==Id::ParamOrthogonal?"(u0+t*u1).(v0+t*v1)=0":"(u0+t*u1) x (v0+t*v1)=0","u(t)=u0+t*u1, v(t)=v0+t*v1. Es werden alle reellen Loesungen der entstehenden Polynome bis Grad 2 bestimmt. Fuer eine Richtung ist der Nullvektor ausgeschlossen.");v(o,"u0",{1,0,0});v(o,"u1",{0,1,0});v(o,"v0",{1,1,0});v(o,"v1",{});}
  {auto& o=add(Id::ParamLength,6,"Parameter: vorgegebene Laenge","|u0+t*u1|^2=L^2","Ausmultiplizieren ergibt (u1.u1)*t^2+2*(u0.u1)*t+(u0.u0-L^2)=0. L muss nichtnegativ sein.");v(o,"u0",{1,0,0});v(o,"u1",{0,1,0});k(o,"L",2);}
  {auto& o=add(Id::ParamPlane,6,"Parameter: P(t) in Ebene","n.(P0+t*w)=d","P(t)=P0+t*w. Die lineare Gleichung hat eine, keine oder fuer jedes t eine Loesung.");v(o,"P0",{1,2,3});v(o,"w",{0,0,1});plane(o,"n","d",{0,0,1},0);}
  {auto& o=add(Id::ParamLine,6,"Parameter: P(t) auf Gerade","P0+t*w=A+s*u","Die unbekannten Parameter t und s werden gemeinsam aus drei Koordinatengleichungen bestimmt. Auch konstante P(t) und unendlich viele Loesungen sind moeglich.");v(o,"P0",{0,1,0});v(o,"w",{0,-1,0});line(o,"A","u",{}, {1,0,0});}
  {auto& o=add(Id::ParamCoplanar,6,"Parameter: vier Punkte koplanar","((B-A) x (C-A)).(D0+t*w-A)=0","A,B,C sind fest, D(t)=D0+t*w. Fuer kollineare A,B,C sind alle vier Punkte fuer jedes t koplanar.");v(o,"A",{});v(o,"B",{1,0,0});v(o,"C",{0,1,0});v(o,"D0",{0,0,1});v(o,"w",{0,0,1});}
  for(int n=2;n<=4;++n){auto& o=add(n==2?Id::Lgs2:n==3?Id::Lgs3:Id::Lgs4,7,("LGS "+std::to_string(n)+" x "+std::to_string(n)).c_str(),"A*x=b; Gauss-Jordan-Verfahren","Gib jede Zeile als Koeffizienten und rechte Seite b ein. Nach Zeilenskalierung und Pivotwahl wird eliminiert. Widerspruch: keine Loesung; freie Variablen: unendlich viele Loesungen.");for(int i=0;i<n;++i){for(int j=0;j<n;++j)k(o,("Z"+std::to_string(i+1)+": a"+std::to_string(j+1)).c_str(),i==j?1:0);k(o,("Z"+std::to_string(i+1)+": b").c_str(),i+1);}}
  {auto& o=add(Id::Combination,7,"Linearkombination bestimmen","a*u+b*v+c*w=p","Die Vektoren bilden die Spalten einer Matrix. Das LGS liefert alle moeglichen Koeffizienten oder zeigt, dass p nicht in der linearen Huelle liegt.");v(o,"u",{1,0,0});v(o,"v",{0,1,0});v(o,"w",{0,0,1});v(o,"p",{2,3,4});}
  {auto& o=add(Id::Independence,7,"Lineare Unabhaengigkeit / Basis","a*u+b*v+c*w=0","Drei Vektoren im R3 sind genau dann eine Basis, wenn Rang=3 bzw. det(u,v,w) ungleich null ist. Bei Abhaengigkeit wird eine nichttriviale Relation angegeben.");two(o);v(o,"w",{0,0,1});}
  return a;
 }();return ops;
}
const Operation& operation(Id id){for(const auto& o:operations())if(o.id==id)return o;return operations().front();}

std::vector<std::string> lesson(const Operation& op,bool example){
 std::vector<std::string> t={op.title,"FORMEL",op.formula,"VERFAHREN",op.method};
 if(op.category==3)t.push_back("Alle Geraden und Ebenen werden im R3 behandelt. 2D-Aufgaben kannst du mit z=0 eingeben. Ebenenform: n.X=d, nicht n.X+d=0; gegebenenfalls das Vorzeichen von d anpassen.");
 if(op.category==6)t.push_back("t ist ein reeller Parameter. Er wird ueber die konstanten Vektoren und ihre t-Koeffizienten eingegeben. Die Eingabefelder erwarten Zahlen, keine freien symbolischen Ausdruecke.");
 if(example){t.push_back("DURCHGERECHNETES BEISPIEL");std::vector<double> x;for(const auto& f:op.fields){x.push_back(f.initial);t.push_back(f.label+" = "+number(f.initial));}auto r=calculate(op,x);t.insert(t.end(),r.text.begin(),r.text.end());}
 return t;
}
}
