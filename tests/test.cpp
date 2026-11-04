#include "app.hpp"
#include "screen_backup.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>

using namespace mathe;
int checks=0;
void check(bool ok){++checks;if(!ok){std::fprintf(stderr,"Check %d failed\n",checks);std::abort();}}
bool close(double a,double b,double rel=1e-8){return std::abs(a-b)<=rel*std::max({1.0,std::abs(a),std::abs(b)});}
void eq(Vec a,Vec b){check(close(a.x,b.x)&&close(a.y,b.y)&&close(a.z,b.z));}
Report calc(Id id,std::initializer_list<double> v){return calculate(operation(id),v);}
bool has(const Report& r,const std::string& s){for(const auto& t:r.text)if(t.find(s)!=std::string::npos)return true;return false;}
bool has(const std::vector<std::string>& r,const std::string& s){for(const auto& t:r)if(t.find(s)!=std::string::npos)return true;return false;}
void snapshot(App& a,const char* name){std::vector<uint16_t> p(320*528);Canvas c(p.data());a.draw(c);std::ofstream f(std::string("docs/preview/")+name+".ppm",std::ios::binary);f<<"P6\n320 528\n255\n";for(uint16_t v:p){char b[3]={static_cast<char>(((v>>11)&31)*255/31),static_cast<char>(((v>>5)&63)*255/63),static_cast<char>((v&31)*255/31)};f.write(b,3);}check(f.good());}
void tap(App& a,int x,int y){a.touchDown(x,y);a.touchUp();}
void coreTests(){
 for(auto t:std::vector<std::pair<std::string,double>>{{"1/3",1.0/3},{"sqrt(2)^2",2},{"-2^2",-4},{"(-2)^2",4},{"2^-3",.125},{"2^3^2",512},{"1E-3",.001},{"2,5+1",3.5},{"pi",pi},{"(2+3)/4",1.25}}){auto p=parse(t.first);check(p.ok&&close(p.value,t.second));}
 for(const char* s:{"","1/0","sqrt(-1)","2a","1e","1..2","pi2","1e13","1e-13","(1+2","nan","2**3","(-2)^0.5","1e-999","(1e-12)^100"})check(!parse(s).ok);
 check(number(9999999999.9)=="1e+10");check(number(.0001)=="0.0001");check(number(-0.0)=="0");
 check(!parse(std::string(97,'1')).ok);
 std::mt19937 rng(42);std::uniform_real_distribution<double> d(-10,10);
 for(int i=0;i<400;++i){Vec a{d(rng),d(rng),d(rng)},b{d(rng),d(rng),d(rng)},c{d(rng),d(rng),d(rng)};Vec n=cross(a,b);check(close(dot(n,a),0)&&close(dot(n,b),0));eq(cross(b,a),n*-1);check(close(dot(cross(a,b),c),dot(a,cross(b,c))));check(close(norm(unit(a)),1));eq((a+b)-b,a);}
 auto roots=quadratic(1,-3,2);check(roots.values.size()==2&&close(roots.values[0],1)&&close(roots.values[1],2));check(quadratic(0,0,0).all);check(quadratic(0,0,1).values.empty());check(quadratic(1,0,1).values.empty());check(quadratic(1,2,1).values.size()==1);roots=quadratic(1,0,-1e24);check(roots.values.size()==2&&close(roots.values[0],-1e12)&&close(roots.values[1],1e12));
 std::array<std::array<double,5>,4> a{};a[0]={1,1,3};a[1]={2,-1,0};auto r=solve(a,2,2,true);check(r.consistent&&r.rank==2&&r.basis.empty()&&close(r.particular[0],1)&&close(r.particular[1],2));
 a={};a[0]={1,2,3};a[1]={2,4,6};r=solve(a,2,2);check(r.consistent&&r.rank==1&&r.basis.size()==1);check(close(r.particular[0]+2*r.particular[1],3));check(close(r.basis[0][0]+2*r.basis[0][1],0));
 a[1][2]=7;r=solve(a,2,2);check(!r.consistent);
 a={};a[0]={0,0,1e-12};r=solve(a,2,2);check(!r.consistent);
 a={};a[0]={1e-12,0,1e-12};a[1]={0,1e12,2e12};r=solve(a,2,2);check(r.consistent&&r.rank==2&&close(r.particular[0],1)&&close(r.particular[1],2));
 for(int n=2;n<=4;++n)for(int sample=0;sample<100;++sample){a={};std::array<double,4>x{};for(int j=0;j<n;++j)x[j]=d(rng);for(int i=0;i<n;++i){for(int j=0;j<n;++j){a[i][j]=d(rng);if(i==j)a[i][j]+=40;a[i][n]+=a[i][j]*x[j];}}r=solve(a,n,n);check(r.consistent&&r.rank==n);for(int j=0;j<n;++j)check(close(r.particular[j],x[j]));}
 auto p=linePair({0,0,0},{1,0,0},{0,1,0},{0,-1,0});check(p.kind==LinePair::Intersect&&close(p.t,1));eq(p.p,{0,0,0});
 p=linePair({0,0,0},{1,0,0},{0,0,2},{0,1,0});check(p.kind==LinePair::Skew&&close(p.distance,2));
 p=linePair({0,0,0},{1,0,0},{1,2,0},{-2,0,0});check(p.kind==LinePair::Parallel&&close(p.distance,2));
 p=linePair({0,0,0},{1,0,0},{2,0,0},{-2,0,0});check(p.kind==LinePair::Identical&&close(p.distance,0));
 p=linePair({0,0,0},{1,0,0},{1e12,1,0},{-2,0,0});check(p.kind==LinePair::Parallel&&close(p.distance,1));
 p=linePair({0,0,0},{1,0,0},{1e12,0,1},{0,1,0});check(p.kind==LinePair::Skew&&close(p.distance,1));
 check(!parallel({1e12,1,0},{1e12,0,0}));
 for(int i=0;i<300;++i){Vec a0{d(rng),d(rng),d(rng)},u{d(rng),d(rng),d(rng)},b{d(rng),d(rng),d(rng)},v{d(rng),d(rng),d(rng)};p=linePair(a0,u,b,v);check(close(dot(p.p-p.q,unit(u)),0,1e-7)&&close(dot(p.p-p.q,unit(v)),0,1e-7));check(close(norm(p.p-p.q),p.distance));}
}
void operationTests(){
 check(operations().size()>=45);
  for(const auto& op:operations()){std::vector<double> x;for(const auto& f:op.fields)x.push_back(f.initial);auto r=calculate(op,x);if(!r.ok)std::fprintf(stderr,"Default failed: %s\n",op.title.c_str());check(r.ok&&r.text.size()>4);for(const auto& s:r.text)check(s.find("[Zahlenbereich ueberschritten]")==std::string::npos);check(lesson(op,true).size()>r.text.size());}
 check(has(calc(Id::Dot,{1,2,3,2,-1,1}),"u.v = 3"));
 check(!calc(Id::Normalize,{0,0,0}).ok);
 check(!calc(Id::Angle,{0,0,0,1,0,0}).ok);
 check(!calc(Id::LineThrough,{1,2,3,1,2,3}).ok);
 check(!calc(Id::PlaneThrough,{0,0,0,1,0,0,2,0,0}).ok);
 check(!calc(Id::Divide,{0,0,0,1,0,0,1,-1}).ok);
 check(has(calc(Id::Midpoint,{0,0,0,2,4,6}),"M=(A+B)/2 = (1; 2; 3)"));
 check(has(calc(Id::Triangle,{0,0,0,4,0,0,0,3,0}),"F=|n|/2 = 6"));
 check(has(calc(Id::Tetrahedron,{0,0,0,1,0,0,0,2,0,0,0,3}),"V=|det|/6 = 1"));
 check(has(calc(Id::TrianglePoint,{0,0,0,4,0,0,0,3,0,1,1,0}),"Inneren"));
 check(has(calc(Id::TrianglePoint,{0,0,0,4,0,0,0,3,0,1,1,1}),"ausserhalb der Dreiecksebene"));
 check(has(calc(Id::TrianglePoint,{0,0,0,4,0,0,0,3,0,5,1,0}),"ausserhalb des Dreiecks"));
 check(has(calc(Id::TrianglePoint,{0,0,0,4,0,0,0,3,0,2,0,0}),"Rand"));
 check(has(calc(Id::PointLine,{1,2,0,0,0,0,1,0,0}),"Spiegelpunkt P'=2F-P = (1; -2; 0)"));
 check(has(calc(Id::PointPlane,{1,2,3,0,0,1,1}),"Abstand |h| = 2"));
 check(has(calc(Id::LinePlane,{0,0,0,1,0,0,0,0,1,0}),"liegt in der Ebene"));
 check(has(calc(Id::LinePlane,{0,0,2,1,0,0,0,0,1,0}),"echt parallel"));
 check(has(calc(Id::LinePlane,{0,0,2,0,0,-1,0,0,1,0}),"S=A+t*u bzw. P(t) = (0; 0; 0)"));
 check(has(calc(Id::PlanePlane,{0,0,1,2,0,0,-2,-4}),"identisch"));
 check(has(calc(Id::PlanePlane,{0,0,1,2,0,0,-2,-6}),"Abstand = 1"));
 check(has(calc(Id::PlanePlane,{0,0,1,0,0,1,0,2}),"Schnittgerade"));
 check(!calc(Id::PointPlane,{1,2,3,0,0,0,0}).ok);
 check(has(calc(Id::SphereLine,{0,0,0,1,-2,0,0,1,0,0}),"Sekante"));
 check(has(calc(Id::SphereLine,{0,0,0,1,-2,1,0,1,0,0}),"Beruehrpunkt"));
 check(has(calc(Id::SphereLine,{0,0,0,1,-2,2,0,1,0,0}),"Passante"));
 check(has(calc(Id::SphereLine,{0,0,0,1,1e12,0,0,1,0,0}),"Sekante"));
 check(has(calc(Id::SpherePlane,{0,0,0,1,0,0,1,1}),"Beruehrpunkt"));
 check(has(calc(Id::SpherePlane,{0,0,0,1,0,0,1,2}),"Kein Schnitt"));
 check(has(calc(Id::SphereSphere,{0,0,0,2,0,0,0,2}),"Identische Kugeln"));
 check(has(calc(Id::SphereSphere,{0,0,0,2,4,0,0,2}),"beruehren"));
 check(has(calc(Id::SphereSphere,{0,0,0,2,1,0,0,1}),"beruehren"));
 check(has(calc(Id::SphereSphere,{0,0,0,3,1,0,0,1}),"Kein gemeinsamer"));
 check(has(calc(Id::SphereSphere,{0,0,0,2,3,0,0,2}),"Kreisradius"));
 check(has(calc(Id::SphereFour,{1,0,0,0,1,0,0,0,1,-1,0,0}),"r=|M-A| = 1"));
 check(has(calc(Id::SphereFour,{0,0,0,1,0,0,2,0,0,3,0,0}),"Keine Loesung"));
 check(has(calc(Id::ParamOrthogonal,{1,0,0,0,1,0,1,1,0,0,0,0}),"t1 = -1"));
 check(has(calc(Id::ParamParallel,{1,0,0,0,1,0,1,1,0,0,0,0}),"t1 = 1"));
 check(has(calc(Id::ParamParallel,{0,0,0,1,0,0,1,0,0,0,0,0}),"Ausgeschlossen: t=0"));
 check(has(calc(Id::ParamParallel,{0,0,0,0,0,0,1,0,0,0,0,0}),"Keine reelle Loesung"));
 check(has(calc(Id::ParamLength,{0,0,0,1,0,0,1e12}),"t1 = -1e+12"));
 check(has(calc(Id::ParamPlane,{0,0,0,0,0,0,0,0,1,0}),"Alle reellen"));
 check(has(calc(Id::ParamLine,{0,1,0,0,-1,0,0,0,0,1,0,0}),"P(t) = (0; 0; 0)"));
 check(has(calc(Id::ParamLine,{0,0,0,0,0,0,0,0,0,1,0,0}),"Unendlich viele"));
 check(has(calc(Id::ParamCoplanar,{0,0,0,1,0,0,0,1,0,0,0,1,0,0,1}),"t1 = -1"));
 check(!calc(Id::SpherePoint,{0,0,0,-1,1,0,0}).ok);
}
void uiTests(){
 App a;snapshot(a,"01-home");tap(a,30,100);check(a.page==App::Page::Categories);a.key(Down);snapshot(a,"02-topics");a.key(Up);a.key(Enter);check(a.page==App::Page::Operations);snapshot(a,"03-operations");a.key(Enter);check(a.page==App::Page::Form);snapshot(a,"04-input");
 a.key('1');a.key('/');a.key('0');a.key(Enter);check(a.page==App::Page::Form&&!a.error.empty());snapshot(a,"05-invalid");a.key(127);a.key('2');a.key(Enter);check(a.error.empty()&&a.field==1);a.key('=');check(a.page==App::Page::Text);snapshot(a,"06-result");check(has(a.document,"Ergebnis = (2.5; 1; 4)"));
 a.key(Enter,true);check(a.page==App::Page::Text);a.key(Back);check(a.page==App::Page::Form&&a.inputs[0]=="1/2");a.key(Clear);check(a.page==App::Page::Operations);a.key(Back);a.key(Back);check(a.page==App::Page::Home);
 a.selected=3;a.key(Enter);a.key(Enter);check(a.planar);a.key(Back);a.selected=0;a.key(Enter);a.key(Enter);a.key(Enter);check(a.page==App::Page::Form&&a.inputs[2]=="0");a.key(Enter);a.key(Enter);check(a.field==3);a.key('=');snapshot(a,"07-2d");check(has(a.document,"Ergebnis = (3; 1; 0)"));
 a.key(Back);a.key(Back);a.key(Back);a.key(Back);a.selected=1;a.key(Enter);a.key(Enter);a.key(Enter);check(a.page==App::Page::Text);snapshot(a,"08-theory");a.key(Right);check(a.scroll>0);int before=a.scroll;a.touchDown(100,200);a.touchMove(100,120);a.touchUp();check(a.scroll>=before);a.key(Back);check(a.page==App::Page::Operations);
 App b;b.touchDown(20,100);b.touchMove(310,400);b.touchUp();check(b.page==App::Page::Home);b.key(Enter,true);check(b.page==App::Page::Home);b.key(Clear);check(!b.running);
 std::vector<uint16_t> pixels(320*528);Canvas c(pixels.data());c.rect(-100,-100,1000,1000,0);c.text(-50,515,"Text clipped",0xffff);check(!c.wrap({std::string(200,'W')},290).empty());check(c.width(c.fit("Sehr langer Titel zur Kontrolle",100))<=100);
}
void openForm(App& app,Id id){
 app.key(Enter);app.selected=operation(id).category;app.key(Enter);
 int index=0;for(const auto& op:operations()){if(op.category!=operation(id).category)continue;if(op.id==id)break;++index;}
 app.selected=index;app.key(Enter);check(app.page==App::Page::Form&&app.current->id==id);
}
void groupedInputTests(){
 auto groups=inputGroups(operation(Id::Add),false);
 check(groups.size()==2&&groups[0].title=="Vektor 1 (u)"&&groups[1].title=="Vektor 2 (v)");
 check(groups[0].fields==std::vector<int>({0,1,2})&&groups[1].fields==std::vector<int>({3,4,5}));
 check(groups[0].tone!=groups[1].tone&&groupForField(groups,4)==1);
 auto flat=inputGroups(operation(Id::Add),true);
 check(flat[0].fields==std::vector<int>({0,1})&&flat[1].fields==std::vector<int>({3,4}));
 check(groupForField(flat,2)==-1&&groupForField(flat,5)==-1);
 check(inputGroups(operation(Id::Lgs2),false).empty());
 auto sphere=inputGroups(operation(Id::SpherePoint),false);
 check(sphere.size()==3&&sphere[0].title=="Mittelpunkt M"&&sphere[1].title=="Radius r"&&sphere[2].title=="Punkt P");
 for(const auto& op:operations())for(bool planar:{false,true}){
  auto grouped=inputGroups(op,planar);if(grouped.empty())continue;
  std::vector<int> seen(op.fields.size());
  for(const auto& group:grouped){check(!group.fields.empty()&&group.fields.size()<=3);for(int i:group.fields){check(i>=0&&i<static_cast<int>(seen.size()));++seen[i];}}
  for(size_t i=0;i<seen.size();++i){const auto& label=op.fields[i].label;bool hidden=planar&&op.planar&&label.size()>2&&label.substr(label.size()-2)==".z";check(seen[i]==(hidden?0:1));}
 }
 App a;openForm(a,Id::Add);tap(a,150,211);check(a.field==4);snapshot(a,"09-active-vector2");
 a.key('1');a.key('/');a.key('0');tap(a,40,142);check(a.field==4&&!a.error.empty());
 a.key(127);a.key('2');tap(a,40,142);check(a.field==0&&a.inputs[4]=="1/2");
 tap(a,260,211);check(a.field==5);tap(a,40,103);check(a.field==0);
 App two;two.planar=true;openForm(two,Id::Add);tap(two,230,142);check(two.field==1);two.key(Enter);check(two.field==3);tap(two,230,211);check(two.field==4);snapshot(two,"10-input-2d");
 App three;openForm(three,Id::Triple);for(int i=0;i<6;++i)three.key(Enter);check(three.field==6);snapshot(three,"11-input-three-vectors");
 tap(three,260,211);check(three.field==8);tap(three,258,58);check(three.field==0);tap(three,292,58);check(three.field==3);tap(three,40,173);check(three.field==6);
 App scalar;openForm(scalar,Id::Scale);tap(scalar,260,211);check(scalar.field==3);snapshot(scalar,"12-input-scalar");scalar.key('3');scalar.key('=');check(has(scalar.document,"k*u = (3; 6; 9)"));
 App lgs;openForm(lgs,Id::Lgs2);tap(lgs,200,178);check(lgs.field==2);
}
void rapidClearTests(){
 App app;openForm(app,Id::Add);
 app.key(Clear);check(app.running&&app.page==App::Page::Operations);
 app.key(Clear,true);check(app.running&&app.page==App::Page::Operations);
 app.key(Clear);check(app.running&&app.page==App::Page::Categories);
 app.key(Clear);check(app.running&&app.page==App::Page::Home);
 app.key(Clear,true);check(app.running&&app.page==App::Page::Home);
 app.key(Clear);check(!app.running);app.key(Clear);check(!app.running);
 App result;openForm(result,Id::Add);result.key('=');check(result.page==App::Page::Text);
 for(auto expected:{App::Page::Form,App::Page::Operations,App::Page::Categories,App::Page::Home}){result.key(Clear);check(result.running&&result.page==expected);}
 result.key(Clear);check(!result.running);
 App invalid;openForm(invalid,Id::Add);invalid.key('1');invalid.key('/');invalid.key('0');invalid.key(Enter);check(!invalid.error.empty());
 for(int i=0;i<4;++i)invalid.key(Clear);
 check(!invalid.running);
}
void inputHelpTests(){
 for(const auto& op:operations())for(bool planar:{false,true})for(size_t i=0;i<op.fields.size();++i){
  auto text=inputHelp(op,static_cast<int>(i),planar);
  check(text.size()>=8&&has(text,"Bedeutung:")&&has(text,"Woher nehmen / herleiten:")&&has(text,"Beispiel:"));
  check(!has(text,"Fuer dieses Feld fehlt")&&has(text,op.fields[i].label));
 }
 check(inputHelp(operation(Id::Add),-1,false).size()==1);
 check(inputHelp(operation(Id::Add),6,false).size()==1);
 check(has(inputHelp(operation(Id::PointLine),6,false),"Richtungsvektor u"));
 check(has(inputHelp(operation(Id::PointLine),6,false),"Q-P"));
 check(has(inputHelp(operation(Id::PointLine),7,false),"u.y im Beispiel -1"));
 check(has(inputHelp(operation(Id::PointLine),8,false),"u.z im Beispiel 4"));
 check(has(inputHelp(operation(Id::PlaneParam),3,false),"Spannvektor u"));
 check(has(inputHelp(operation(Id::PlaneParam),6,false),"Spannvektor v"));
 check(has(inputHelp(operation(Id::PlaneParam),7,false),"v.y im Beispiel 1"));
 check(has(inputHelp(operation(Id::LineLine),6,false),"zweiten Geraden h"));
 check(has(inputHelp(operation(Id::LineLine),9,false),"h: X=B+t*v"));
 check(has(inputHelp(operation(Id::PointPlane),4,false),"n.y im Beispiel -1"));
 check(has(inputHelp(operation(Id::LinePlane),9,false),"rechts -d0"));
 check(has(inputHelp(operation(Id::LinePlane),9,false),"d=6 eingeben"));
 check(has(inputHelp(operation(Id::SpherePoint),1,false),"M.y im Beispiel -2"));
 check(has(inputHelp(operation(Id::SpherePoint),3,false),"r=sqrt(16)=4, nicht 16"));
 check(has(inputHelp(operation(Id::SphereSphere),3,false),"ersten Kugel K1"));
 check(has(inputHelp(operation(Id::SphereSphere),7,false),"zweiten Kugel K2"));
 check(has(inputHelp(operation(Id::ParamOrthogonal),4,false),"u1.y im Beispiel -3"));
 check(has(inputHelp(operation(Id::ParamOrthogonal),5,false),"u1.z im Beispiel 0"));
 check(has(inputHelp(operation(Id::ParamParallel),11,false),"v1.z im Beispiel 1"));
 check(has(inputHelp(operation(Id::ParamCoplanar),13,false),"D(t)=D0+t*w"));
 check(has(inputHelp(operation(Id::ParamLine),9,false),"g: X=A+s*u"));
 check(has(inputHelp(operation(Id::Divide),7,false),"Teilverhaeltnis n"));
 check(has(inputHelp(operation(Id::Lgs4),18,false),"Koeffizient von x4 in Z4"));
 check(has(inputHelp(operation(Id::Lgs4),19,false),"Rechte Seite in Z4"));
 check(has(inputHelp(operation(Id::Add),0,true),"2D-Eingabe"));
 check(!has(inputHelp(operation(Id::LinePlane),0,true),"2D-Eingabe"));

 App draft;openForm(draft,Id::Add);tap(draft,150,211);draft.key('1');draft.key('/');
 auto saved=draft.inputs;int savedField=draft.field;
 tap(draft,295,247);check(draft.page==App::Page::Text&&draft.inputs==saved&&draft.field==savedField&&draft.error.empty());
 check(has(draft.document,"Eingabe v.y")&&has(draft.document,"Vektor v"));
 draft.key(Clear);check(draft.page==App::Page::Form&&draft.running&&draft.inputs==saved&&draft.field==savedField);
 draft.key('2');check(draft.inputs[4]=="1/2");
 draft.key('?');check(draft.page==App::Page::Text);draft.key(Enter);check(draft.page==App::Page::Form&&draft.field==4);
 draft.key(127);draft.key('0');draft.key(Enter);check(!draft.error.empty());
 auto savedError=draft.error;saved=draft.inputs;
 tap(draft,295,247);check(draft.page==App::Page::Text&&draft.inputs==saved&&draft.error==savedError);
 draft.key(Back);check(draft.page==App::Page::Form&&draft.error==savedError&&draft.field==4);

 App initial;openForm(initial,Id::Scale);tap(initial,260,211);tap(initial,295,247);
 check(initial.page==App::Page::Text&&has(initial.document,"Faktor k"));
 initial.key(Back);initial.key('5');check(initial.inputs[3]=="5");
 App direction;openForm(direction,Id::PointLine);for(int i=0;i<6;++i)direction.key(Enter);
 snapshot(direction,"14-help-button");auto coordinates=direction.inputs;tap(direction,295,247);
 check(direction.page==App::Page::Text&&has(direction.document,"Richtungsvektor u"));snapshot(direction,"13-help-direction");
 direction.key(Right);check(direction.scroll>0);direction.key(Clear);
 check(direction.page==App::Page::Form&&direction.field==6&&direction.inputs==coordinates);
 tap(direction,260,211);check(direction.field==8);
 App plane;openForm(plane,Id::LinePlane);for(int i=0;i<9;++i)plane.key(Enter);tap(plane,295,247);snapshot(plane,"15-help-plane");
 check(has(plane.document,"Rechte Seite d"));plane.key(Clear);check(plane.field==9&&plane.page==App::Page::Form);
 App lgs;openForm(lgs,Id::Lgs2);tap(lgs,200,157);tap(lgs,295,247);snapshot(lgs,"16-help-lgs");
 check(lgs.page==App::Page::Text&&has(lgs.document,"Koeffizient von x2 in Z1"));
 lgs.key(Clear);check(lgs.page==App::Page::Form&&lgs.field==1);
 for(int i=0;i<4;++i)lgs.key(Clear);
 check(!lgs.running);
}
namespace {
int allocations=0, releases=0;
void* allocateScreen(std::size_t size){++allocations;check(size==ScreenBackup::bytes);return std::malloc(size);}
void releaseScreen(void* memory){++releases;std::free(memory);}
void* failAllocation(std::size_t){++allocations;return nullptr;}
}
void screenTests(){
 std::vector<unsigned char> pixels(ScreenBackup::bytes);
 for(size_t i=0;i<pixels.size();++i)pixels[i]=static_cast<unsigned char>(i*31u+7u);
 const auto original=pixels;
 ScreenBackup backup;
 check(!backup.ready());check(!backup.restore());
 check(!backup.save(nullptr,allocateScreen,releaseScreen));
 check(!backup.save(pixels.data(),nullptr,releaseScreen));
 check(!backup.save(pixels.data(),allocateScreen,nullptr));
 check(allocations==0&&releases==0);
 check(!backup.save(pixels.data(),failAllocation,releaseScreen));
 check(!backup.ready()&&!backup.restore()&&pixels==original&&releases==0);
 check(backup.save(pixels.data(),allocateScreen,releaseScreen));
 check(backup.ready()&&pixels==original);
 check(!backup.save(pixels.data(),allocateScreen,releaseScreen));
 std::fill(pixels.begin(),pixels.end(),0x55);
 check(backup.restore());check(pixels==original&&!backup.ready());
 check(!backup.restore()&&releases==1);
 check(backup.save(pixels.data(),allocateScreen,releaseScreen));
 std::fill(pixels.begin(),pixels.end(),0xaa);
 check(backup.restore()&&pixels==original&&allocations==3&&releases==2);
}
int main(){coreTests();operationTests();uiTests();groupedInputTests();rapidClearTests();inputHelpTests();screenTests();std::printf("PASS: %d checks; %zu operations; 16 UI snapshots.\n",checks,operations().size());}
