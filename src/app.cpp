#include "app.hpp"
#include "font.hpp"
#include <algorithm>
#include <cstdlib>

namespace mathe {
namespace {
constexpr uint16_t bg=0xef9d,fg=0x1144,accent=0x2490,select=0xb6b6,white=0xffff,dim=0x6b6d,line=0xcdf7,red=0xa800;
constexpr int header=44, rowHeight=48, footer=488;
constexpr uint16_t groupColors[]={0x2490,0x22f2,0x9c20,0x704e};
constexpr uint16_t groupBackgrounds[]={0xe79c,0xe77f,0xff37,0xef3e};
uint16_t groupColor(const InputGroup& group){return group.tone<0?dim:groupColors[group.tone];}
uint16_t groupBackground(const InputGroup& group){return group.tone<0?0xe71c:groupBackgrounds[group.tone];}
int cardTop(int slot){return 100+slot*70;}
int componentWidth(int count){return (290-6*(count-1))/count;}
const char* keypad[5][5]={{"7","8","9","/","DEL"},{"4","5","6","*","-"},{"1","2","3","+","^"},{"0",".","(",")","sqrt("},{"pi","E","<",">","OK"}};
bool isZ(const std::string& s){return s.size()>2&&s.substr(s.size()-2)==".z";}
}
void Canvas::rect(int x,int y,int w,int h,uint16_t color){int right=std::min(screenWidth,x+w),bottom=std::min(screenHeight,y+h);for(int yy=std::max(0,y);yy<bottom;++yy)for(int xx=std::max(0,x);xx<right;++xx)pixels[yy*screenWidth+xx]=color;}
int Canvas::width(const std::string& s)const{int w=0;for(unsigned char c:s)w+=classic::font::advance[c>=32?c-32:0];return w;}
void Canvas::text(int x,int y,const std::string& s,uint16_t color){for(unsigned char c:s){int k=c>=32?c-32:0;for(int yy=0;yy<20;++yy){int py=y+yy;if(py<0||py>=screenHeight)continue;uint16_t bits=classic::font::bits[k][yy];for(int xx=0;xx<16;++xx)if((bits&(0x8000>>xx))&&x+xx>=0&&x+xx<screenWidth)pixels[py*screenWidth+x+xx]=color;}x+=classic::font::advance[k];}}
std::string Canvas::fit(const std::string& s,int w)const{if(width(s)<=w)return s;std::string out=s;while(!out.empty()&&width(out+"...")>w)out.pop_back();return out+"...";}
std::vector<std::string> Canvas::wrap(const std::vector<std::string>& paragraphs,int w)const{
 std::vector<std::string> result;
 for(const auto& p:paragraphs){std::string current,word;auto flush=[&](){if(word.empty())return;if(!current.empty()&&width(current+" "+word)>w){result.push_back(current);current.clear();}while(width(word)>w){size_t n=1;while(n<word.size()&&width(word.substr(0,n+1))<=w)++n;if(!current.empty()){result.push_back(current);current.clear();}result.push_back(word.substr(0,n));word.erase(0,n);}if(!word.empty()){if(!current.empty())current+=' ';current+=word;}word.clear();};for(char ch:p){if(ch==' '||ch=='\n'){flush();if(ch=='\n'){result.push_back(current);current.clear();}}else word+=ch;}flush();if(!current.empty())result.push_back(current);result.push_back("");}
 return result;
}
App::App()=default;
std::vector<int> App::opIndices()const{std::vector<int> out;const auto& ops=operations();for(size_t i=0;i<ops.size();++i)if(ops[i].category==category)out.push_back(static_cast<int>(i));return out;}
std::vector<int> App::visibleFields()const{std::vector<int> out;if(!current)return out;for(size_t i=0;i<current->fields.size();++i)if(!(planar&&current->planar&&isZ(current->fields[i].label)))out.push_back(static_cast<int>(i));return out;}
std::vector<std::string> App::menu()const{
 if(page==Page::Home)return {"Vektor-Rechner","Theorie & Beispiele","Formelsammlung","Einstellungen","Bedienung & Hinweise","Beenden"};
 if(page==Page::Categories)return categories();
 if(page==Page::Operations){std::vector<std::string> a;for(int i:opIndices())a.push_back(operations()[i].title);return a;}
 if(page==Page::Settings)return {planar?"Vektoreingabe: 2D":"Vektoreingabe: 3D","Zahlen: 10 signifikante Stellen","Zurueck"};
 return {};
}
void App::ensureVisible(){int count=static_cast<int>(menu().size());if(!count)return;selected=std::clamp(selected,0,count-1);if(selected<first)first=selected;if(selected>=first+8)first=selected-7;first=std::clamp(first,0,std::max(0,count-8));}
void App::ensureFormGroupVisible(const std::vector<InputGroup>& groups){
 int active=groupForField(groups,field);
 if(active<0)return;
 if(active<firstFormGroup)firstFormGroup=active;
 if(active>=firstFormGroup+2)firstFormGroup=active-1;
 firstFormGroup=std::clamp(firstFormGroup,0,std::max(0,static_cast<int>(groups.size())-2));
}
void App::openText(const std::string& title,const std::vector<std::string>& paragraphs){returnPage=page;returnSelection=selected;textTitle=title;document=paragraphs;scroll=0;page=Page::Text;dirty=true;}
void App::showInputHelp(){if(page==Page::Form&&current)openText("Hilfe zu "+current->fields[field].label,inputHelp(*current,field,planar));}
void App::activate(){
 if(page==Page::Home){switch(selected){
  case 0:case 1:case 2:mode=selected;page=Page::Categories;selected=0;break;
  case 3:page=Page::Settings;selected=0;break;
  case 4:openText("Bedienung",{
   "MATHE VEKTOR v1.0.3 - ClassPad II / HH3",
   "Touch: Eintrag antippen. Pfeile: Auswahl; EXE: oeffnen. Clear oder Zurueck: eine Ebene zurueck. Auf dem Startbildschirm beendet Clear die App direkt und fuehrt zum Launcher.",
   "Eingabe: ein Zahlenfeld pro Komponente. Beim ersten Tippen wird der vorgegebene Wert ersetzt. DEL loescht; links/rechts wechselt zum vorherigen/naechsten Feld. EXE bestaetigt und geht weiter. Das letzte EXE oder Rechnen startet die Rechnung.",
   "Vektoren stehen in getrennten farbigen Bloecken. x, y und z direkt antippen; der aktive Vektor und die aktive Komponente sind markiert. Bei mehr als zwei Bloecken oben mit < und > blaettern. EXE fuehrt automatisch zum naechsten Feld und Block.",
   "Das ? neben dem aktiven Eingabefeld erklaert seine Bedeutung, die Herkunft bzw. Herleitung und ein Zahlenbeispiel. Zurueck oder Clear kehrt mit denselben Werten und demselben aktiven Feld zur Eingabe zurueck. Die Hilfe ist auch bei unvollstaendiger Eingabe erreichbar.",
   "Erlaubt: + - * / ^, Klammern, sqrt(...), pi, E fuer Zehnerpotenzen. Beispiel: 1/3, sqrt(2), -2^2=-4, (2+3)/4, 1E-3. Dezimalkomma und Dezimalpunkt werden akzeptiert.",
   "Zahlenfelder erwarten numerische Ausdruecke. Parameter werden ueber feste Vektoren und ihre t-Koeffizienten eingegeben. Allgemeine symbolische Terme werden nicht geloest.",
   "2D/3D unter Einstellungen betrifft markierte Vektor- und Punktaufgaben. In 2D ist z=0. Geraden, Ebenen, Kugeln und raeumliche Aufgaben nutzen immer 3D.",
   "Gerade: X=A+t*u; u muss ungleich null sein. Ebene: n.X=d. Bei ax+by+cz+d=0 muss die rechte Seite -d eingegeben werden.",
   "Die Ergebnisse werden mit Gleitkommazahlen berechnet und auf 10 signifikante Stellen dargestellt. Brueche und Wurzeln werden numerisch ausgewertet. Relative Vergleichstoleranz: 1e-10. Nahe entarteten Faellen koennen Rundungsfehler die Fallentscheidung beeinflussen.",
   "Eingabebereich: null oder Betrag zwischen 1e-12 und 1e12. Bei stark unterschiedlichen Groessenordnungen zuerst sinnvoll skalieren.",
   "Textseiten: oben/unten scrollt eine Zeile, links/rechts eine Seite. Ziehen scrollt. Unten sind Tasten fuer Vor/Zurueck und Rueckkehr. Aus dem Rechenweg kommst du zu deinen Eingaben zurueck.",
   "Jede Aufgabe hat eine Formel, Erklaerung und ein durchgerechnetes Beispiel unter Theorie & Beispiele. Alle Funktionen arbeiten offline; die App benoetigt nur mathe.hh3."
  });break;
  case 5:running=false;break;
 }first=0;}
 else if(page==Page::Categories){category=selected;page=Page::Operations;selected=0;first=0;}
 else if(page==Page::Operations){auto indices=opIndices();current=&operations()[indices[selected]];opSelection=selected;if(mode==0)beginForm();else openText(current->title,lesson(*current,mode==1));}
 else if(page==Page::Settings){if(selected==0)planar=!planar;else if(selected==1)openText("Zahlenformat",{"Berechnung mit double, Anzeige mit 10 signifikanten Stellen. Brueche und Wurzeln werden numerisch ausgewertet.","Nahezu singulaere Gleichungssysteme und fast parallele Richtungen sind numerisch empfindlich. Die relative Vergleichstoleranz betraegt 1e-10."});else back();}
 dirty=true;
}
void App::beginForm(){inputs.clear();for(const auto& f:current->fields)inputs.push_back(number(planar&&current->planar&&isZ(f.label)?0:f.initial));field=0;first=0;firstFormGroup=0;error.clear();replaceOnType=true;page=Page::Form;}
bool App::validate(int i){auto p=parse(inputs[i]);if(p.ok){error.clear();return true;}field=i;error=p.error;replaceOnType=false;dirty=true;return false;}
void App::compute(){std::vector<double> values;for(size_t i=0;i<inputs.size();++i){if(!validate(static_cast<int>(i)))return;values.push_back(parse(inputs[i]).value);}auto r=calculate(*current,values);openText(r.ok?"Ergebnis & Rechenweg":"Eingabe pruefen",r.text);}
void App::edit(const std::string& s){if(replaceOnType){inputs[field].clear();replaceOnType=false;}if(inputs[field].size()+s.size()<=96)inputs[field]+=s;else error="Maximal 96 Zeichen.";dirty=true;}
void App::back(){
 if(page==Page::Text){page=returnPage;selected=returnSelection;}
 else if(page==Page::Form){page=Page::Operations;selected=opSelection;first=0;error.clear();}
 else if(page==Page::Operations){page=Page::Categories;selected=category;first=0;}
 else if(page==Page::Categories||page==Page::Settings){page=Page::Home;selected=page==Page::Settings?3:mode;first=0;}
 else {running=false;}
 dirty=true;
}
void App::key(int code,bool repeat){
 if(!running)return;
 if(repeat&&(code==Enter||code==Clear||code==Back||code=='='))return;
 if(code==Clear||code==Back){back();return;}
 if(page==Page::Text){if(code==Enter){back();return;}int delta=code==Up?-1:code==Down?1:code==Left?-18:code==Right?18:0;scroll=std::clamp(scroll+delta,0,std::max(0,textLines-20));dirty=true;return;}
 if(page==Page::Form){
  if(code=='?'){showInputHelp();return;}
  if(code=='='&&!repeat){compute();return;}
  if(code==127){if(replaceOnType){inputs[field].clear();replaceOnType=false;}else if(!inputs[field].empty())inputs[field].pop_back();error.clear();dirty=true;return;}
  if(code==Up||code==Down||code==Left||code==Right||code==Enter){if(!validate(field))return;auto fields=visibleFields();auto it=std::find(fields.begin(),fields.end(),field);int pos=static_cast<int>(it-fields.begin());int direction=code==Up||code==Left?-1:1;if(code==Enter&&pos==static_cast<int>(fields.size())-1){compute();return;}field=fields[std::clamp(pos+direction,0,static_cast<int>(fields.size())-1)];replaceOnType=true;dirty=true;return;}
  if(code>=32&&code<127){const std::string allowed="0123456789.,+-*/^()eEpisqrt";if(allowed.find(static_cast<char>(code))!=std::string::npos){error.clear();edit(std::string(1,static_cast<char>(code)));}}return;
 }
 if(code==Up||code==Down||code==Left||code==Right){int step=code==Up?-1:code==Down?1:code==Left?-8:8;selected+=step;ensureVisible();dirty=true;}
 else if(code==Enter)activate();
}
void App::touchDown(int x,int y){if(x<0||x>=320||y<0||y>=528)return;touchX=x;touchY=y;moveY=y;touchCancelled=false;drag=false;startScroll=page==Page::Text?scroll:first;}
void App::touchMove(int x,int y){if(touchX<0)return;if(x<0||x>=320||y<0||y>=528){touchCancelled=true;return;}if(page==Page::Text&&touchY>=64&&touchY<footer&&std::abs(y-touchY)>10){drag=true;scroll=std::clamp(startScroll+(touchY-y)/20,0,std::max(0,textLines-20));dirty=true;}else if(page!=Page::Form&&page!=Page::Text&&touchY>=74&&touchY<footer&&std::abs(y-touchY)>12){drag=true;auto m=menu();first=std::clamp(startScroll+(touchY-y)/rowHeight,0,std::max(0,static_cast<int>(m.size())-8));selected=std::clamp(selected,first,std::min(first+7,static_cast<int>(m.size())-1));dirty=true;}else if(std::abs(x-touchX)>12||std::abs(y-touchY)>12)touchCancelled=true;moveY=y;}
void App::touchUp(){if(touchX>=0&&!touchCancelled&&!drag)tap(touchX,touchY);touchX=touchY=-1;}
void App::tap(int x,int y){
 if(y<header&&x<62&&page!=Page::Home){back();return;}
 if(page==Page::Text){if(y>=footer){if(x<100)key(Left);else if(x<200)key(Right);else back();}return;}
 if(page==Page::Form){
  if(x>=278&&x<312&&y>=235&&y<259){showInputHelp();return;}
  if(y>=footer){if(x<106)back();else if(x<213)key(Enter);else compute();return;}
  auto groups=inputGroups(*current,planar);
  if(!groups.empty()){
   ensureFormGroupVisible(groups);
   if(groups.size()>2&&y>=46&&y<70&&x>=242&&x<312){
    if(!validate(field))return;
    firstFormGroup=std::clamp(firstFormGroup+(x<278?-2:2),0,static_cast<int>(groups.size())-2);
    field=groups[firstFormGroup].fields.front();replaceOnType=true;dirty=true;return;
   }
   for(int slot=0;slot<2&&firstFormGroup+slot<static_cast<int>(groups.size());++slot){
    const auto& group=groups[firstFormGroup+slot];int top=cardTop(slot),target=-1;
    if(y>=top&&y<top+24&&x>=7&&x<313)target=groupForField(groups,field)==firstFormGroup+slot?field:group.fields.front();
    int w=componentWidth(static_cast<int>(group.fields.size()));
    for(size_t j=0;j<group.fields.size();++j){int left=16+static_cast<int>(j)*(w+6);if(y>=top+27&&y<top+58&&x>=left&&x<left+w)target=group.fields[j];}
    if(target>=0){if(validate(field)){field=target;replaceOnType=true;dirty=true;}return;}
   }
  }else{
   auto fields=visibleFields();int pos=static_cast<int>(std::find(fields.begin(),fields.end(),field)-fields.begin());int start=std::clamp(pos-1,0,std::max(0,static_cast<int>(fields.size())-4));
   if(y>=112&&y<232){int index=start+(y-112)/30;if(index<static_cast<int>(fields.size())&&validate(field)){field=fields[index];replaceOnType=true;dirty=true;}return;}
  }
  if(y>=334&&y<484&&x>=7&&x<312){int col=(x-7)/61,row=(y-334)/30;if(col>=5||row>=5)return;std::string s=keypad[row][col];if(s=="DEL")key(127);else if(s=="<")key(Left);else if(s==">")key(Right);else if(s=="OK")key(Enter);else {error.clear();edit(s);}return;}
  return;
 }
 if(y>=footer){if(x<100)key(Left);else if(x<200)key(Right);else if(page!=Page::Home)back();return;}
 if(y>=80&&y<464){int index=first+(y-80)/rowHeight;auto m=menu();if(index<static_cast<int>(m.size())){selected=index;activate();}}
}
void App::draw(Canvas& c){
 c.rect(0,0,320,528,bg);c.rect(0,0,320,header,accent);
 std::string title=page==Page::Home?"Mathe Vektor":page==Page::Categories?mode==0?"Rechner":mode==1?"Theorie & Beispiele":"Formelsammlung":page==Page::Operations?categories()[category]:page==Page::Settings?"Einstellungen":page==Page::Form?current->title:textTitle;
 if(page!=Page::Home)c.text(9,12,"<",white);
 c.text(page==Page::Home?12:36,12,c.fit(title,page==Page::Home?295:275),white);
 c.rect(0,footer,320,40,select);
 if(page==Page::Text){auto rows=c.wrap(document,294);textLines=static_cast<int>(rows.size());scroll=std::clamp(scroll,0,std::max(0,textLines-20));c.text(10,49,"Zeile "+std::to_string(scroll+1)+" / "+std::to_string(textLines),dim);for(int i=0;i<20&&scroll+i<textLines;++i)c.text(10,78+i*20,rows[scroll+i],fg);c.text(14,499,"< Seite",fg);c.text(110,499,"Seite >",fg);c.text(222,499,"Zurueck",fg);if(textLines>20){c.rect(312,78,3,400,line);int h=std::max(12,400*20/textLines);int y=78+(400-h)*scroll/std::max(1,textLines-20);c.rect(312,y,3,h,accent);}dirty=false;return;}
 if(page==Page::Form){
  auto groups=inputGroups(*current,planar);
  uint16_t activeColor=accent;
  std::string context;
  if(!groups.empty()){
   ensureFormGroupVisible(groups);
   int active=groupForField(groups,field);
   if(active>=0){activeColor=groupColor(groups[active]);context=groups[active].title;if(groups[active].vector)context+=" / "+std::string(1,current->fields[field].label.back());}
   std::string dimension=planar&&current->planar?"2D":"3D";
   if(groups.size()>2){
    c.text(9,48,dimension+" | Block "+std::to_string(firstFormGroup+1)+"-"+std::to_string(std::min(firstFormGroup+2,static_cast<int>(groups.size())))+"/"+std::to_string(groups.size()),dim);
    c.rect(242,46,32,24,white);c.rect(280,46,32,24,white);c.text(253,48,"<",fg);c.text(290,48,">",fg);
   }else c.text(9,48,dimension+" | Komponenten antippen",dim);
   for(int slot=0;slot<2&&firstFormGroup+slot<static_cast<int>(groups.size());++slot){
    const auto& group=groups[firstFormGroup+slot];int top=cardTop(slot);uint16_t color=groupColor(group);
    c.rect(7,top,306,64,color);c.rect(11,top+1,301,62,groupBackground(group));
    c.text(18,top+2,c.fit(group.title,285),color);
    int w=componentWidth(static_cast<int>(group.fields.size()));
    for(size_t j=0;j<group.fields.size();++j){
     int i=group.fields[j],left=16+static_cast<int>(j)*(w+6);bool activeField=i==field;
     c.rect(left,top+27,w,31,color);c.rect(left+1,top+28,w-2,29,activeField?color:white);
     std::string label=(group.vector?std::string(1,current->fields[i].label.back())+": ":"")+inputs[i];label=c.fit(label,w-10);
     c.text(left+(w-c.width(label))/2,top+32,label,activeField?white:fg);
    }
   }
  }else{
   c.text(9,48,"EXE: naechstes Feld",dim);
   auto fields=visibleFields();int pos=static_cast<int>(std::find(fields.begin(),fields.end(),field)-fields.begin());int start=std::clamp(pos-1,0,std::max(0,static_cast<int>(fields.size())-4));
   for(int j=0;j<4&&start+j<static_cast<int>(fields.size());++j){int i=fields[start+j],y=112+j*30;if(i==field)c.rect(6,y,307,29,select);c.text(12,y+4,c.fit(current->fields[i].label,135),fg);c.text(154,y+4,c.fit(inputs[i],149),fg);}
   context="Feld "+std::to_string(pos+1)+"/"+std::to_string(fields.size())+": "+current->fields[field].label;
  }
  c.text(9,74,c.fit(current->formula,302),fg);
  c.text(9,237,c.fit(context,257),activeColor);
  c.rect(278,235,34,24,activeColor);c.text(290,237,"?",white);
  c.rect(7,262,305,34,white);c.rect(7,294,305,2,activeColor);std::string value=inputs[field];while(!value.empty()&&c.width(value+"_")>287)value.erase(0,1);if(replaceOnType)c.rect(11,268,std::min(290,c.width(value)+4),22,select);c.text(12,269,value+"_",fg);
  c.text(9,304,c.fit(error.empty()?"DEL: loeschen | Clear: zurueck":error,302),error.empty()?dim:red);
  for(int row=0;row<5;++row)for(int col=0;col<5;++col){int x=7+col*61,y=334+row*30;c.rect(x,y,59,28,white);std::string s=keypad[row][col];c.text(x+(59-c.width(s))/2,y+4,s,fg);}
  c.text(8,499,"Zurueck",fg);c.text(116,499,"Weiter",fg);c.text(225,499,"Rechnen",fg);dirty=false;return;
 }
 auto m=menu();ensureVisible();
 std::string hint=page==Page::Home?"Schule / Abitur  |  v1.0.3":page==Page::Settings?"Antippen oder EXE zum Aendern":"Auswahl "+std::to_string(selected+1)+" / "+std::to_string(m.size());c.text(10,51,hint,dim);
 for(int j=0;j<8&&first+j<static_cast<int>(m.size());++j){int i=first+j,y=80+j*rowHeight;if(i==selected)c.rect(0,y,320,rowHeight,select);c.rect(0,y+rowHeight-1,320,1,line);auto wrapped=c.wrap({m[i]},273);if(wrapped.size()>2){c.text(12,y+4,wrapped[0],fg);c.text(12,y+24,c.fit(wrapped[1],273),fg);}else c.text(12,y+14,c.fit(m[i],273),fg);c.text(298,y+14,">",dim);}
 c.text(13,499,"< Seite",fg);c.text(110,499,"Seite >",fg);c.text(223,499,page==Page::Home?(planar?"2D":"3D"):"Zurueck",fg);dirty=false;
}
}
