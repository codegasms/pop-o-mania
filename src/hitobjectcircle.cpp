#include <hitobject.hpp>
#include <cmath>
#include <algorithm>
#include "gamemanager.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include <limits>

Circle::Circle(HitObjectData data){
    this->data = data;
    init();
}

void Circle::init(){
    data.ex = data.x;
    data.ey = data.y;
}

void Circle::update(){
    GameManager* gm = GameManager::getInstance();
    if(gm->currentTime*1000.0f > data.time + gm->gameFile.p50Final){
        data.time = gm->currentTime*1000.0f;
        data.point = 0;
        if(gm->clickCombo > 30){
            SetSoundVolume(gm->SoundFilesAll.data["combobreak"], 1.0f);
            PlaySound(gm->SoundFilesAll.data["combobreak"]);
        }
        gm->clickCombo = 0;
        gm->destroyHitObject(data.index);
    }
}

void Circle::render(){
    GameManager* gm = GameManager::getInstance();
    float approachScale = 3.5f*easeInOutCubic((1-(gm->currentTime*1000.0f - data.time + gm->gameFile.preempt)/gm->gameFile.preempt))+1.0f;
    if (approachScale <= 1.0f)
        approachScale = 1.0f;
    float clampedFade = (gm->currentTime*1000.0f - data.time  + gm->gameFile.preempt) / gm->gameFile.fade_in;

    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade);
    

    DrawTextureCenter(gm->hitCircle, data.x, data.y, gm->circlesize/(float)Global.textureSize.hitCircle , renderColor);
    DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/(float)Global.textureSize.comboNumber, Fade(WHITE,clampedFade));
    DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, gm->circlesize/(float)Global.textureSize.hitCircleOverlay , Fade(WHITE,clampedFade));
    DrawTextureCenter(gm->approachCircle, data.x, data.y, approachScale*(gm->circlesize/(float)Global.textureSize.approachCircle) , renderColor);

}

void Circle::dead_render(){
    GameManager* gm = GameManager::getInstance();
    float clampedFade = (gm->gameFile.fade_in/1.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/1.0f);
    float clampedFade2 = (gm->gameFile.fade_in/4.0f + data.time - gm->currentTime*1000.0f) / (gm->gameFile.fade_in/4.0f);

    clampedFade = clip(clampedFade, 0.0f, 1.0f);
    clampedFade = easeInOutCubic(clampedFade);

    clampedFade2 = clip(clampedFade2, 0.0f, 1.0f);
    clampedFade2 = easeInOutCubic(clampedFade2);

    float scale = (gm->currentTime*1000.0f + gm->gameFile.fade_in/2.0f - data.time) / (gm->gameFile.fade_in/2.0f);
    scale = clip(scale,1,2);
    scale -= 1.0;
    scale = easeOutQuad(scale);
    scale = scale * 0.4f;
    scale += 1.0f;
    
    Color renderColor;
    if(data.colour.size() > 2)
        renderColor =  Fade(Color{(unsigned char)data.colour[0],(unsigned char)data.colour[1],(unsigned char)data.colour[2]}, clampedFade2);
    else
        renderColor =  Fade(Color{255,255,255}, clampedFade2);
    DrawTextureCenter(gm->hitCircle, data.x, data.y, clip(scale,1,2)*gm->circlesize/(float)Global.textureSize.hitCircle , renderColor);
    DrawCNumbersCenter(data.comboNumber, data.x, data.y, gm->circlesize/(float)Global.textureSize.comboNumber, Fade(WHITE,clampedFade2));
    DrawTextureCenter(gm->hitCircleOverlay, data.x, data.y, clip(scale,1,2)*gm->circlesize/(float)Global.textureSize.hitCircleOverlay , Fade(WHITE,clampedFade2));
    /*if(data.point != 0)
        DrawTextureCenter(gm->selectCircle, data.x, data.y, scale*gm->circlesize/gm->selectCircle.width*(gm->selectCircle.width/128.0f) , renderColor);*/
    if(data.point == 0)
        DrawTextureCenter(gm->hit0, data.x, data.y, (gm->circlesize/gm->hit0.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 1)
        DrawTextureCenter(gm->hit50, data.x, data.y, (gm->circlesize/gm->hit50.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 2)
        DrawTextureCenter(gm->hit100, data.x, data.y, (gm->circlesize/gm->hit100.width)*0.7f , Fade(WHITE,clampedFade));
    else if(data.point == 3 && Global.textureSize.render300)
        DrawTextureCenter(gm->hit300, data.x, data.y, (gm->circlesize/gm->hit300.width)*0.7f , Fade(WHITE,clampedFade));
}

void Circle::dead_update(){
    GameManager* gm = GameManager::getInstance();
    if (data.time+gm->gameFile.fade_in/1.0f < gm->currentTime*1000.0f){
        data.expired = true;
    }
}
