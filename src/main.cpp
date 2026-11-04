#include "app.hpp"
#include "screen_backup.hpp"
#include <appdef.h>
#include <sdk/os/input.h>
#include <sdk/os/lcd.h>

APP_NAME("Mathe Vektor")
APP_DESCRIPTION("Vektorrechnung und analytische Geometrie mit Rechenwegen. Schule / Abitur.")
APP_AUTHOR("Loewenkrieger")
APP_VERSION("1.0.3")

int main(){
 if(!mathe::screenReady())return 1;
 unsigned int w=0,h=0;LCD_GetSize(&w,&h);
 if(w!=mathe::screenWidth||h!=mathe::screenHeight)return 0;
 mathe::Canvas canvas(LCD_GetVRAMAddress());
 mathe::App app;
 while(app.running){
  if(app.dirty){app.draw(canvas);LCD_Refresh();}
  alignas(4) Input_Event event{};GetInput(&event,0xffffffff,0x10);
  if(event.type==EVENT_ACTBAR_ESC){app.key(mathe::Back);continue;}
  if(event.type==EVENT_KEY&&(event.data.key.direction==KEY_PRESSED||event.data.key.direction==KEY_HELD)){
   int k=static_cast<int>(event.data.key.keyCode),mapped=0;bool repeat=event.data.key.direction==KEY_HELD;
   switch(k){
    case KEYCODE_UP:mapped=mathe::Up;break;case KEYCODE_DOWN:mapped=mathe::Down;break;
    case KEYCODE_LEFT:mapped=mathe::Left;break;case KEYCODE_RIGHT:mapped=mathe::Right;break;
    case KEYCODE_EXE:mapped=mathe::Enter;break;case KEYCODE_POWER_CLEAR:mapped=mathe::Clear;break;
    case KEYCODE_BACKSPACE:mapped=127;break;case KEYCODE_NEGATIVE:mapped='-';break;
    case KEYCODE_TIMES:mapped='*';break;case KEYCODE_EXP:mapped='E';break;
    default:if(k>=32&&k<127)mapped=k;break;
   }
   if(mapped)app.key(mapped,repeat);
  }else if(event.type==EVENT_TOUCH){auto t=event.data.touch_single;
   if(t.direction==TOUCH_DOWN)app.touchDown(t.p1_x,t.p1_y);
   else if(t.direction==TOUCH_HOLD_DRAG)app.touchMove(t.p1_x,t.p1_y);
   else if(t.direction==TOUCH_UP)app.touchUp();
  }
 }
 return 0;
}
