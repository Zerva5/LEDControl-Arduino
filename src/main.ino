#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define startCommand 's'
#define timeCommand 't'
#define itemAddCommand 'i'
#define colorAddCommand 'a'
#define colorCommand 'c'
#define indexCommand 'p'
#define directionCommand 'd'
#define lengthCommand 'l'

#define commandIndex 0
#define itemIndex 1
#define colorIndex 2
#define rgbIndex 3
#define valueIndex 4

bool newCommand = false;

const int numChars = 33;
char receivedChars[33];


//Contains color data and the specific attributes of that color
struct colorid {
    //char
    int rgb[3];
    int len;
};

typedef struct colorid colorID;

struct item {
    colorID * colors;
    //num
    int colorCount;
    int t;
    int maxt;
    int curIndex;
    int direction;
};

typedef struct item Item;

struct ledStrip{
    int active;
    int itemCount;
    Item * items;
};

typedef struct ledStrip LEDStrip;

//Function declarations
int ChangeItemCount(int newItemCount);
void ChangeColorCount(Item * item, int newColorCount);
void Strip_close(LEDStrip * oldStrip);
int Item_close(Item *);
void Color_close(colorID *);
void Strip_init(LEDStrip * newStrip, int itemCount);
void Item_init(Item * newItem, int colorCount);
void Color_init(colorID * newColor, bool randomColor);
void LED_default(LEDStrip * stripItem);
void getCommand();
int parseCommand();

//All strips used in the program, need to recompile to add a new strip.
int LEDCOUNT = 299;
LEDStrip Strip;
int startIndex = 0;

int itemCount_d = 1;
int colorCount_d = 1;

Adafruit_NeoPixel STRIP (LEDCOUNT, 13, NEO_GRB + NEO_KHZ800);


void setup() {
    //Zero out receivedChars cause idk this might help :(
    receivedChars[0] = '\0';

    //Initialize strips
    Strip_init(&Strip, itemCount_d);
    for(int i = 0; i < Strip.itemCount; i = i + 1){
        Item_init(&Strip.items[i], colorCount_d);
    }

    Strip.items[0].colors[0].rgb[1] = 255;

    Strip.items[0].curIndex = 10;
    Strip.items[0].colors[0].rgb[2] = 0;
    Strip.items[0].colors[0].rgb[1] = 0;
    

//    Serial.begin(9600);
//    Serial.println("<Ready For Command>");

    STRIP.begin();

//    Serial.println(Strip.items[0].colors[0].rgb[0]);
//    Serial.println(Strip.items[0].colors[0].rgb[1]);
//    Serial.println(Strip.items[0].colors[0].rgb[2]);


    LED_default(&Strip);


//    for(int i = 0; i < StripCount; i = i + 1){
//        Strip_close(&Strip);
//    }
}

void loop(){
  getCommand();
  if(newCommand == true){
    parseCommand();
    newCommand = false;
//    LED_default(&STRIP, 5);
  }

//    Serial.println(Strip.items[0].colors[0].test);
    LED_default(&Strip);
    delay(50);
//    Serial.println(Strip.items[0].t);
  
    
}

void LED_default(LEDStrip * stripItem){
    STRIP.clear();
//
    for(int i = 0; i < stripItem->itemCount; i++){

        int currentIndex = stripItem->items[i].curIndex;

        for(int c = 0; c < stripItem->items[i].colorCount; c = c + 1){
            
            uint32_t color = STRIP.Color(stripItem->items[i].colors[c].rgb[0], stripItem->items[i].colors[c].rgb[1], stripItem->items[i].colors[c].rgb[2]);
            STRIP.fill(color, currentIndex, stripItem->items[i].colors[c].len);
            
            currentIndex = currentIndex + stripItem->items[i].colors[c].len;
        }

        if(stripItem->active == 1 && stripItem->items[i].t >= stripItem->items[i].maxt){
            if(stripItem->items[i].direction == 1){
                stripItem->items[i].curIndex++;
            }else{
                stripItem->items[i].curIndex--;
            }
            stripItem->items[i].t = 0;
        }else{
            stripItem->items[i].t++;
        }
        if(stripItem->items[i].curIndex > LEDCOUNT){
            stripItem->items[i].curIndex = 0;
        }
        if(stripItem->items[i].curIndex < 0){
            stripItem->items[i].curIndex = LEDCOUNT;
        }
    }
    
    STRIP.show(); 
}

int parseCommand(){

    int item = 0;
    int color = 0;
    int rgb = 0;
    int value = 0;

    //Init strings for all of the seperate command values
    char itemStr[1];
    char colorStr[1];
    char rgbStr[1];
    char valueStr[3];


    itemStr[0] = receivedChars[itemIndex];
    colorStr[0] = receivedChars[colorIndex];
    rgbStr[0] = receivedChars[rgbIndex];

    valueStr[0] = receivedChars[valueIndex];
    valueStr[1] = receivedChars[valueIndex + 1];
    valueStr[3] = receivedChars[valueIndex + 2];

    item = strtol(itemStr, NULL, 16);
    color = strtol(colorStr, NULL, 16);
    rgb = strtol(rgbStr, NULL, 16);
    value = strtol(valueStr, NULL, 16);

    switch (receivedChars[0])
    {
        case colorCommand:
            if(value <= 255 && value >= 0){
                Strip.items[item].colors[color].rgb[rgb] = value;
            }
            break;
        case lengthCommand:
            Strip.items[item].colors[color].len = value;
            break;
        case timeCommand:
            Strip.items[item].maxt = value;
            break;
        case indexCommand:
            if(value < LEDCOUNT && value >= 0){
                Strip.items[item].curIndex = value;
            }
            break;
        case startCommand:
            if(value == 0 || value == 1){
                Strip.active = value;
            }
            break;
        case itemAddCommand:
            ChangeItemCount(value);
            break;
        case colorAddCommand:
            ChangeColorCount(&Strip.items[item], value);
            break;
        case directionCommand:
            if(value == 0 || value == 1){
                Strip.items[item].direction = value;
            }
        default:
            break;
    }
    
    
}

void getCommand() {
    static boolean recvInProgress = false;
    static byte recvIndex = 0;
    char startMark = '<';
    char endMark = '>';
    char recv;
 
    while (Serial.available() > 0 && newCommand == false) {
        recv = Serial.read();

        if (recvInProgress == true) {
            if (recv != endMark) {
              
                receivedChars[recvIndex] = recv;
                recvIndex++;

                //May have to bump up numChars
                if (recvIndex >= numChars) {
                    recvIndex = numChars - 1;
                }
                
            }
            else {
                receivedChars[recvIndex] = '\0'; // terminate the string
                
                recvInProgress = false;
                recvIndex = 0;
                newCommand = true;
            }
        }

        else if (recv == startMark) {
            recvInProgress = true;
        }
    }
}

int ChangeItemCount(int newItemCount){
    int lengthChange = newItemCount - Strip.itemCount;
    // You have 1 item "items[0]"
    // you want to add an item, 
    // 1. realoc the item array
    // 2. init the new items but not the old ones
    // 3. make sure the strip knows it has new items.

    // remove an item, you have 3, you want 2
    // 1. close the old items "items[2]"
    // 2. realoc array
    // change item count for strip

    if(lengthChange > 0){
        Strip.items = (Item *)realloc(Strip.items, sizeof(Item) * newItemCount);
        for (int i = 0; i < lengthChange; i++) {
            // itemCount = 2
            // max index items[1]
            Item_init(&Strip.items[i + Strip.itemCount], colorCount_d);
        }
    }else if(lengthChange < 0){
        for (int i = 0; i < (lengthChange * -1); i++) {
            Item_close(&Strip.items[Strip.itemCount - i]);
        }
        Strip.items = (Item *)realloc(Strip.items, sizeof(Item) * newItemCount);
    }

    Strip.itemCount = newItemCount;
    return 0;
}

void ChangeColorCount(Item * item, int newColorCount){
    int lengthChange = newColorCount - item->colorCount;
    printf("change: %i\n", lengthChange);

    if(lengthChange > 0){
        item->colors = (colorID *)realloc(item->colors, sizeof(colorID) * newColorCount);
        for (int i = 0; i < lengthChange; i++) {
            Color_init(&item->colors[i + item->colorCount], false);
        }
    }else if(lengthChange < 0){

        for(int i = 0; i < (lengthChange * -1); i++){
            Color_close(&item->colors[item->colorCount - i - 1]);        
        }

        item->colors = (colorID *)realloc(item->colors, sizeof(colorID) * newColorCount);
    }
    item->colorCount = newColorCount;
//    Serial.printlm(item->colorCount
}

void Color_close(colorID * oldColor){
    //No need to close colors, no malloc happened
    //Here just in case things change and I can edit this instead of my other functions
}

int Item_close(Item * oldItem){
    free(oldItem->colors);
    Serial.println("closed item");
    return 0;
}

void Strip_close(LEDStrip * oldStrip){
    for (int i = 0; i < oldStrip->itemCount; i = i + 1) {
        Item_close(&oldStrip->items[i]);
    }
    free(oldStrip->items);
}

void Strip_init(LEDStrip * newStrip, int itemCount){

    newStrip->active = 1;

//    newStrip->LEDType = default;

    newStrip->itemCount = itemCount;

    newStrip->items = (Item *)malloc(sizeof(Item) * itemCount);


//    Color_init(&newStrip->items[0].colors[0], 3);
//    newStrip->items[0].colors[0].len = 11;
    //    Item_init(&newStrip->items[0], 3);

//    Serial.println(newStrip->items[0].colors[0].len);
//    Serial.println(Strip.items[0].colors[0].len);

//


}

void Item_init(Item * newItem, int colorCount){
//    Serial.println("HELLO");

    newItem->t = 0;
    newItem->maxt = 10;

    newItem->direction = 1;

    newItem->curIndex = 0;

    newItem->colorCount = colorCount;

    newItem->colors = (colorID *)malloc(sizeof(colorID) * colorCount);

    for(int i = 0; i < colorCount; i = i + 1){
        Color_init(&newItem->colors[i], false);
    }
}

void Color_init(colorID * newColor, bool randomColor){
    int newLen = 2;
    newColor->len = newLen;

    if(randomColor == true){
        newColor->rgb[0] = 0;
        newColor->rgb[1] = 0;
        newColor->rgb[2] = 0;
    }else{
        newColor->rgb[0] = 255;
        newColor->rgb[1] = 255;
        newColor->rgb[2] = 255;
    }

}
