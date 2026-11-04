#include "input_help.hpp"

namespace mathe {
namespace {
struct Explanation {
  std::string title, meaning, source, example;
  Vec vector{};
  double scalar = 0;
};

std::string exampleVector(Vec v, bool planar) {
  return planar ? "("+number(v.x)+"; "+number(v.y)+")" : vectorText(v);
}

Explanation point(const Operation& op, const std::string& name, bool planar) {
  Explanation e;
  e.title="Punkt "+name;e.vector={1,2,planar?0.0:3.0};
  e.meaning="Die Koordinaten des Punkts "+name+" im Koordinatensystem.";
  e.source="Lies den Punkt "+name+" aus der Aufgabenstellung oder der beschrifteten Zeichnung ab. Gib seine Koordinaten ein, keinen Differenzvektor.";
  if(op.id==Id::TrianglePoint&&name=="P")e.meaning="Der Punkt P, dessen Lage zum Dreieck ABC geprueft werden soll.";
  else if((op.id==Id::PointLine||op.id==Id::PointPlane)&&name=="P")e.meaning="Der gegebene Punkt P, dessen Abstand, Lotfuss und Spiegelpunkt berechnet werden sollen.";
  else if(op.id==Id::SpherePoint&&name=="P")e.meaning="Der zu pruefende Punkt P. Seine Lage und sein Abstand zur Kugel werden bestimmt.";
  else if(op.id==Id::Triangle||op.id==Id::TrianglePoint)e.meaning="Der Eckpunkt "+name+" des Dreiecks ABC.";
  else if(op.id==Id::Tetrahedron)e.meaning=name=="D"?"Die Spitze D ueber dem Grunddreieck ABC.":"Der Eckpunkt "+name+" des Grunddreiecks ABC.";
  else if(op.id==Id::SphereFour){e.meaning="Einer der vier vorgegebenen Punkte auf der gesuchten Kugeloberflaeche.";e.source="Nimm die Koordinaten des gegebenen Punkts "+name+". Mittelpunkt und Radius berechnet die App daraus.";}
  else if(op.id==Id::PlaneThrough)e.meaning="Einer der drei Punkte A, B und C, durch die die Ebene verlaufen soll.";
  e.example=name+exampleVector(e.vector,planar)+": erster Eintrag x, zweiter y"+(planar?std::string("."):std::string(", dritter z."));
  return e;
}

Explanation support(const std::string& name, const std::string& equation, const std::string& role) {
  return {"Stuetzpunkt "+name,role,
    "In "+equation+" ist "+name+" der konstante Punkt vor den Parametertermen. Wenn zwei Punkte gegeben sind, kannst du einen davon als Stuetzpunkt nehmen.",
    "X=(1; 2; 3)+t*(2; -1; 4): Stuetzpunkt "+name+"=(1; 2; 3).",{1,2,3},0};
}

Explanation direction(const std::string& name, const std::string& equation, const std::string& parameter) {
  return {"Richtungsvektor "+name,"Dieser Vektor beschreibt die Richtung der Geraden. Er darf nicht der Nullvektor sein.",
    "In "+equation+" steht "+name+" hinter "+parameter+". Aus zwei Punkten P und Q bekommst du ihn mit Q-P.",
    "X=(1; 2; 3)+"+parameter+"*(2; -1; 4): "+name+"=(2; -1; 4). Aus P=(1; 2; 3), Q=(3; 1; 7) ergibt Q-P denselben Vektor.",{2,-1,4},0};
}

Explanation affine(const std::string& name, bool fourthPoint) {
  const bool vectorFamily=name[0]=='u'||name[0]=='v';
  const bool coefficient=(vectorFamily&&name.back()=='1')||name=="w";
  const bool second=name[0]=='v';
  const std::string family=vectorFamily?name.substr(0,1):fourthPoint?"D":"P";
  Vec base=second?Vec{2,1,3}:vectorFamily?Vec{1,2,4}:Vec{1,3,4};
  Vec change=second?Vec{-2,0,1}:vectorFamily?Vec{1,-3,0}:Vec{2,-1,0};
  std::string expression=second?"(2-2*t; 1; 3+t)":vectorFamily?"(1+t; 2-3*t; 4)":"(1+2*t; 3-t; 4)";
  Explanation e;
  e.title=coefficient?"Parameterkoeffizient "+name:"Konstanter Anteil "+name;
  e.meaning=coefficient?"Die Zahlen vor t in jeder Komponente. Wo kein t vorkommt, steht hier 0.":"Der konstante Anteil der Vektor- oder Punktfamilie, also ihr Wert bei t=0.";
  e.source="Sortiere jede Komponente nach konstantem Anteil und t-Anteil. "+(vectorFamily?family+"(t)="+family+"0+t*"+family+"1.":family+"(t)="+family+"0+t*w.")+" Einen konkreten Wert fuer t musst du nicht raten; den berechnet die App.";
  e.vector=coefficient?change:base;
  e.example=family+"(t)="+expression+": "+name+"="+vectorText(e.vector)+".";
  return e;
}

Explanation coordinateHelp(const Operation& op, const std::string& name, bool planar) {
  // Parameter coefficients must be distinguished from ordinary directions.
  if(op.category==6){
    if(name=="u0"||name=="u1"||name=="v0"||name=="v1"||name=="P0"||name=="D0"||name=="w")return affine(name,op.id==Id::ParamCoplanar);
  }
  if(name=="n"||name=="n1"||name=="n2"){
    std::string plane=name=="n1"?"E1":name=="n2"?"E2":"E";
    return {"Normalenvektor "+name,"Ein Vektor senkrecht zur Ebene "+plane+". Er darf nicht null sein.",
      "Bei "+plane+": a*x+b*y+c*z=d ist "+name+"=(a; b; c). Fehlt eine Variable, ist ihr Koeffizient 0. Aus zwei Spannvektoren bekommst du eine Normale durch deren Kreuzprodukt.",
      plane+": 2*x-y+3*z=6 ergibt "+name+"=(2; -1; 3).",{2,-1,3},0};
  }
  if(name=="M"||name=="M1"||name=="M2"){
    std::string sphere=name=="M1"?"K1":name=="M2"?"K2":"K";
    return {"Mittelpunkt "+name,"Die Koordinaten des Mittelpunkts der Kugel "+sphere+".",
      "Bei (x-a)^2+(y-b)^2+(z-c)^2=r^2 ist der Mittelpunkt (a; b; c). Achte auf das Minus in den Klammern. Bei ausmultiplizierter Form hilft quadratische Ergaenzung: x^2+4*x=(x+2)^2-4.",
      sphere+": (x-1)^2+(y+2)^2+(z-3)^2=16 hat "+name+"=(1; -2; 3).",{1,-2,3},0};
  }
  if(op.id==Id::PlaneParam){
    if(name=="A"){
      auto e=support(name,"E: X=A+s*u+t*v","Der konstante Stuetzpunkt der Ebene.");
      e.example="E: X=(1; 2; 3)+s*(1; 0; 0)+t*(0; 1; 0): A=(1; 2; 3).";return e;
    }
    return {"Spannvektor "+name,"Einer der beiden Vektoren, die die Ebene aufspannen. Die beiden Spannvektoren muessen linear unabhaengig sein.",
      "Bei E: X=A+s*u+t*v steht u hinter s und v hinter t. Sind drei Punkte A, B, C gegeben, kannst du u=B-A und v=C-A nehmen.",
      name=="u"?"E: X=A+s*(1; 0; 0)+t*(0; 1; 0): u=(1; 0; 0).":"E: X=A+s*(1; 0; 0)+t*(0; 1; 0): v=(0; 1; 0).",
      name=="u"?Vec{1,0,0}:Vec{0,1,0},0};
  }
  if(op.id==Id::LineLine){
    if(name=="A")return support(name,"g: X=A+s*u","Der Stuetzpunkt der ersten Geraden g.");
    if(name=="B")return support(name,"h: X=B+t*v","Der Stuetzpunkt der zweiten Geraden h.");
    if(name=="u")return direction(name,"g: X=A+s*u","s");
    if(name=="v")return direction(name,"h: X=B+t*v","t");
  }
  const bool lineTask=op.id==Id::PointLine||op.id==Id::LinePlane||op.id==Id::SphereLine||op.id==Id::ParamLine;
  if(lineTask){
    std::string parameter=op.id==Id::ParamLine?"s":"t";
    if(name=="A")return support(name,"g: X=A+"+parameter+"*u","Der Stuetzpunkt der Geraden g.");
    if(name=="u")return direction(name,"g: X=A+"+parameter+"*u",parameter);
  }
  if(name[0]>='A'&&name[0]<='Z')return point(op,name,planar);
  Vec example{2,-1,planar?0.0:3.0};
  Explanation e{"Vektor "+name,"Der Vektor "+name+", mit dem diese Aufgabe rechnet.",
    "Lies seine Komponenten aus der Aufgabe ab. Ist ein Pfeil von A nach B gemeint, berechne zuerst B-A; Start- und Endpunkt haben diese Reihenfolge.",
    name+"="+exampleVector(example,planar)+".",example,0};
  if(op.id==Id::ProjectVector){
    e.meaning=name=="u"?"Der Vektor u, der in einen parallelen und einen senkrechten Anteil zerlegt wird.":"Die Projektionsrichtung v. Sie darf nicht null sein.";
    e.source=name=="u"?"Nimm den zu zerlegenden Vektor aus der Aufgabenstellung.":"Nimm die vorgegebene Projektionsrichtung. Bei Projektion auf eine Gerade ist das deren Richtungsvektor hinter dem Parameter.";
  }else if(op.id==Id::Combination&&name=="p"){
    e.meaning="Der Zielvektor p, der als a*u+b*v+c*w dargestellt werden soll.";
    e.source="Nimm die rechte Seite der vorgegebenen Vektorgleichung. Die Koeffizienten a,b,c sind gesucht und werden von der App bestimmt.";
  }else if(op.id==Id::Parallelogram){
    e.meaning=name=="w"?"Die dritte Kante des Spats, ausgehend vom selben Eckpunkt wie u und v.":"Eine der beiden Kanten der Grundflaeche, ausgehend vom gemeinsamen Eckpunkt.";
    e.source="Bei gemeinsamem Eckpunkt A und weiteren Eckpunkten B,C,D: u=B-A, v=C-A, w=D-A. Fuer die Grundflaeche werden u und v verwendet.";
  }
  return e;
}

Explanation scalarHelp(const Operation& op, const std::string& label) {
  if(op.id==Id::Lgs2||op.id==Id::Lgs3||op.id==Id::Lgs4){
    const int count=op.id==Id::Lgs2?2:op.id==Id::Lgs3?3:4;
    const bool rhs=label.back()=='b';const int column=rhs?0:label.back()-'1';
    const std::string row=label.substr(0,label.find(':'));
    std::string equation="2*x1-3*x2";if(count>=3)equation+="+4*x3";if(count==4)equation+="+0*x4";equation+="=5";
    const double coefficients[]={2,-3,4,0};double value=rhs?5:coefficients[column];
    return {rhs?"Rechte Seite in "+row:"Koeffizient von x"+std::to_string(column+1)+" in "+row,
      rhs?"Die Zahl rechts vom Gleichheitszeichen dieser Gleichung.":"Die Zahl, die in dieser Zeile vor der entsprechenden Unbekannten steht.",
      "Bringe die Gleichung in die Form a1*x1+...+aN*xN=b. Unbekannte stehen links, die konstante Zahl rechts. Vorzeichen mitnehmen; bei fehlender Unbekannter den Koeffizienten 0 eingeben.",
      "Bei "+equation+" steht in diesem Feld "+number(value)+".",{},value};
  }
  if(label=="d"||label=="d1"||label=="d2"){
    std::string plane=label=="d1"?"E1":label=="d2"?"E2":"E";
    return {"Rechte Seite "+label,"Die Konstante auf der rechten Seite der Ebene "+plane+": n.X="+label+".",
      "Stelle die Ebene als a*x+b*y+c*z=d dar. Bei a*x+b*y+c*z+d0=0 muss rechts -d0 stehen. Bei E: X=A+s*u+t*v gilt d=n.A mit n=u x v.",
      plane+": 2*x-y+3*z-6=0 wird zu 2*x-y+3*z=6. Hier "+label+"=6 eingeben.",{},6};
  }
  if(label=="r"||label=="r1"||label=="r2")return {"Radius "+label,"Der nichtnegative Radius "+std::string(label=="r1"?"der ersten Kugel K1.":label=="r2"?"der zweiten Kugel K2.":"der Kugel K."),
    "Bei |X-M|^2=r^2 steht rechts das Quadrat des Radius. Ziehe die Quadratwurzel. Ist der Durchmesser D gegeben, gilt r=D/2.",
    "(x-1)^2+(y+2)^2+(z-3)^2=16: Radius r=sqrt(16)=4, nicht 16.",{},4};
  if(label=="k")return {"Faktor k","Die Zahl, mit der jede Komponente des Vektors multipliziert wird.",
    "Bei k*u ist k die Zahl vor dem Vektor. Ein negatives k kehrt die Richtung um.","Bei 2*u gibst du k=2 ein.",{},2};
  if(label=="L")return {"Gewuenschte Laenge L","Die vorgegebene nichtnegative Laenge, die u(t) haben soll.",
    "Nimm die Laenge aus der Bedingung |u(t)|=L. Ist stattdessen |u(t)|^2 gegeben, ziehe vorher die Wurzel.","Bei |u(t)|=5 gibst du L=5 ein, nicht 25.",{},5};
  if(label=="m"||label=="n")return {"Teilverhaeltnis "+label,
    label=="m"?"Der erste Anteil m im gerichteten Verhaeltnis AT:TB=m:n.":"Der zweite Anteil n im gerichteten Verhaeltnis AT:TB=m:n.",
    "Lies das Verhaeltnis in genau der Reihenfolge AT:TB ab. Fuer den Mittelpunkt ist m:n=1:1. Negative Anteile erlauben aeussere Teilung; m+n darf nicht 0 sein.",
    "Bei AT:TB=2:3 gibst du m=2 und n=3 ein.",{},label=="m"?2.0:3.0};
  return {"Unbekanntes Feld","Fuer dieses Feld fehlt eine Erklaerung.","","",{},0};
}
}

std::vector<std::string> inputHelp(const Operation& op, int field, bool planar) {
  if(field<0||field>=static_cast<int>(op.fields.size()))return {"Ungueltiges Eingabefeld."};
  const std::string& label=op.fields[field].label;
  const bool coordinate=label.size()>2&&label[label.size()-2]=='.';
  const bool twoDimensional=planar&&op.planar;
  const std::string name=coordinate?label.substr(0,label.size()-2):label;
  auto explanation=coordinate?coordinateHelp(op,name,twoDimensional):scalarHelp(op,label);
  std::vector<std::string> result={"Eingabe "+label+": "+explanation.title,
    "Bedeutung: "+explanation.meaning,"Woher nehmen / herleiten: "+explanation.source,
    "Beispiel: "+explanation.example};
  if(coordinate){
    int axis=label.back()=='x'?0:label.back()=='y'?1:2;
    result.push_back("Fuer "+label+" im Beispiel "+number(explanation.vector[axis])+" eingeben. "+std::string(1,label.back())+" ist der "+std::string(axis==0?"erste":axis==1?"zweite":"dritte")+" Eintrag des Vektors oder Punkts.");
    if(twoDimensional)result.push_back("2D-Eingabe: nur x und y eingeben; z wird auf 0 gesetzt.");
  }else result.push_back("In diesem Beispiel hier "+number(explanation.scalar)+" eingeben.");
  result.push_back("Aufgabe: "+op.title+". Formel: "+op.formula);
  result.push_back("Erlaubt sind Zahlen, Brueche und numerische Ausdruecke wie 1/3 oder sqrt(2). Keine freien Buchstaben oder Parameter in Zahlenfelder eingeben.");
  result.push_back("Zurueck oder Clear kehrt zu deinen unveraenderten Eingaben zurueck.");
  return result;
}
}
