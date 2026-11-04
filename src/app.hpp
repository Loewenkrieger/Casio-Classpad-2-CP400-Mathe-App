#pragma once
#include "catalog.hpp"
#include "input_groups.hpp"
#include "input_help.hpp"
#include <cstdint>

namespace mathe {
constexpr int screenWidth=320, screenHeight=528;
enum Key { Up=256, Down, Left, Right, Enter, Back, Clear };
class Canvas {
public:
 uint16_t* pixels;
 explicit Canvas(uint16_t* p):pixels(p){}
 void rect(int x,int y,int w,int h,uint16_t color);
 void text(int x,int y,const std::string& s,uint16_t color);
 int width(const std::string& s) const;
 std::string fit(const std::string& s,int width) const;
 std::vector<std::string> wrap(const std::vector<std::string>& paragraphs,int width) const;
};
class App {
public:
 enum class Page { Home, Categories, Operations, Form, Text, Settings };
 Page page=Page::Home;
 bool running=true, dirty=true;
 int selected=0, first=0, mode=0, category=0, field=0, scroll=0;
 bool planar=false;
 std::string error;
 App();
 void key(int code, bool repeat=false);
 void touchDown(int x,int y);
 void touchMove(int x,int y);
 void touchUp();
 void draw(Canvas& c);
 std::vector<std::string> menu() const;
 const Operation* current=nullptr;
 std::vector<std::string> inputs;
 std::vector<std::string> document;
 std::string textTitle;
private:
 Page returnPage=Page::Home;
 int returnSelection=0, opSelection=0;
 int touchX=-1,touchY=-1, moveY=0,startScroll=0;
 bool touchCancelled=false,drag=false;
 bool replaceOnType=true;
 int textLines=0;
 int firstFormGroup=0;
 std::vector<int> opIndices() const;
 std::vector<int> visibleFields() const;
 void activate();
 void back();
 void openText(const std::string& title,const std::vector<std::string>& paragraphs);
 void showInputHelp();
 void beginForm();
 void compute();
 bool validate(int i);
 void edit(const std::string& s);
 void ensureVisible();
 void ensureFormGroupVisible(const std::vector<InputGroup>& groups);
 void tap(int x,int y);
};
}
