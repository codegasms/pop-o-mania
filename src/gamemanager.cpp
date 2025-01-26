#include "gamemanager.hpp"

#include <math.h>
#include <sys/time.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <hitobject.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "followpoint.hpp"
#include "fs.hpp"
#include "globals.hpp"
#include "utils.hpp"

float GameManager::clip(float value, float min, float max) {
    return std::min(std::max(value, min), max);
}

GameManager *GameManager::inst_ = NULL;

// get a new gamemanager
GameManager *GameManager::getInstance() {
    if (inst_ == NULL) inst_ = new GameManager();
    return (inst_);
}

// call the initilization
GameManager::GameManager() { init(); }

// initilize the game manager
void GameManager::init() {}

// main game loop
void GameManager::update() {
    // for now the left key on the keyboard plays the map automatically
    // this is pretty useful when you need to quickly test the timings
    // since the bot is always pressing in the correct time window
    if (IsKeyDown(SDL_SCANCODE_LEFT)) {
        Global.useAuto = true;
    } else {
        Global.useAuto = false;
    }

    // current implementation of the backgrounds
    int eventSize = gameFile.events.size();
    for (int i = eventSize - 1; i >= 0; i--) {
        if (gameFile.events[i].startTime <= currentTime * 1000.0f) {
            if (gameFile.events[i].eventType == 0) {
                currentBackgroundTexture = gameFile.events[i].filename;
            }
            gameFile.events.pop_back();
        } else
            break;
    }

    for (int i = lastCurrentTiming; i >= 0; i--) {
        if (gameFile.timingPoints[i].time <= currentTime * 1000.0f + 30.0f) {
            currentTimingSettings.renderTicks = gameFile.timingPoints[i].renderTicks;
            currentTimingSettings.sliderSpeedOverride = 1;
            currentTimingSettings.time = gameFile.timingPoints[i].time;
            double tempBeatLength;
            tempBeatLength = gameFile.timingPoints[i].beatLength;
            // std::cout << "beatLength: " << tempBeatLength << std::endl;
            if (tempBeatLength >= 0) {
                currentTimingSettings.beatLength = tempBeatLength;
                verytempbeat2 = tempBeatLength;
                currentTimingSettings.sliderSpeedOverride = 1;
            }
            if (tempBeatLength < 0) {
                currentTimingSettings.sliderSpeedOverride = (100 / tempBeatLength * (-1));
                currentTimingSettings.beatLength = verytempbeat2;
            }
            currentTimingSettings.meter = gameFile.timingPoints[i].meter;
            currentTimingSettings.sampleSet = gameFile.timingPoints[i].sampleSet;
            currentTimingSettings.sampleIndex = gameFile.timingPoints[i].sampleIndex;
            currentTimingSettings.volume = gameFile.timingPoints[i].volume;
            currentTimingSettings.uninherited = gameFile.timingPoints[i].uninherited;
            currentTimingSettings.effects = gameFile.timingPoints[i].effects;
            // std::cout << "Current Timing Settings: " << currentTimingSettings.time << " " <<
            // currentTimingSettings.sampleSet << " " << currentTimingSettings.sampleIndex <<
            // std::endl;
            lastCurrentTiming = i - 1;
        } else
            break;
    }

    timingSettings tempTiming;
    for (int i = lastTimingLoc; i >= 0; i--) {
        if (gameFile.timingPoints[i].time - gameFile.preempt <= currentTime * 1000.0f) {
            tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
            tempTiming.sliderSpeedOverride = 1;
            tempTiming.time = gameFile.timingPoints[i].time;
            double tempBeatLength;
            tempBeatLength = gameFile.timingPoints[i].beatLength;
            // std::cout << "beatLength: " << tempBeatLength << std::endl;
            if (tempBeatLength >= 0) {
                tempTiming.beatLength = tempBeatLength;
                verytempbeat = tempBeatLength;
                tempTiming.sliderSpeedOverride = 1;
            }
            if (tempBeatLength < 0) {
                tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
                tempTiming.beatLength = verytempbeat;
            }
            tempTiming.meter = gameFile.timingPoints[i].meter;
            tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
            tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
            tempTiming.volume = gameFile.timingPoints[i].volume;
            tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
            tempTiming.effects = gameFile.timingPoints[i].effects;
            timingSettingsForHitObject.push_back(tempTiming);
            lastTimingLoc = i - 1;
        } else
            break;
    }
    if (timingSettingsForHitObject.size() == 0) {
        int i = lastTimingLoc;
        if (gameFile.timingPoints.size() == 0) {
            std::cout << "what the fuck" << std::endl;
        } else {
            tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
            tempTiming.sliderSpeedOverride = 1;
            tempTiming.time = gameFile.timingPoints[i].time;
            double tempBeatLength;
            tempBeatLength = gameFile.timingPoints[i].beatLength;
            if (tempBeatLength >= 0) {
                tempTiming.beatLength = tempBeatLength;
                verytempbeat = tempBeatLength;
                tempTiming.sliderSpeedOverride = 1;
            }
            if (tempBeatLength < 0) {
                tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
                tempTiming.beatLength = verytempbeat;
            }
            tempTiming.meter = gameFile.timingPoints[i].meter;
            tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
            tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
            tempTiming.volume = gameFile.timingPoints[i].volume;
            tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
            tempTiming.effects = gameFile.timingPoints[i].effects;
            timingSettingsForHitObject.push_back(tempTiming);
            lastTimingLoc = i - 1;
        }
    }

    // spawn the hitobjects when their time comes
    int size = gameFile.hitObjects.size();
    for (int i = size - 1; i >= 0; i--) {
        if (gameFile.hitObjects[i].time - gameFile.preempt <= currentTime * 1000.0f) {
            /*if(gameFile.hitObjects[i].type == 2 and gameFile.hitObjects[i].totalLength >
            Global.maxSliderSize){ std::cout << "well fuck this long slider i guess. \n";
                    gameFile.hitObjects.pop_back();
            }*/
            // else{
            spawnHitObject(gameFile.hitObjects[i]);
            if (objects[objects.size() - 1]->data.startingACombo) {
                currentComboIndex++;
                if (gameFile.comboColours.size())
                    currentComboIndex =
                        (currentComboIndex + objects[objects.size() - 1]->data.skipComboColours) %
                        gameFile.comboColours.size();
                combo = 1;
            }
            if (gameFile.comboColours.size())
                objects[objects.size() - 1]->data.colour = gameFile.comboColours[currentComboIndex];
            objects[objects.size() - 1]->data.comboNumber = combo;
            combo++;
            int index = 0;
            for (int amog = 0; amog < timingSettingsForHitObject.size(); amog++) {
                if (timingSettingsForHitObject[amog].time > gameFile.hitObjects[i].time) break;
                index = amog;
            }

            // o şejkilde lişğaksda başka ne uzun biliyor musun bence benim akıllığım terinde
            // -ömer 2022
            objects[objects.size() - 1]->data.timing.beatLength =
                timingSettingsForHitObject[index].beatLength;
            objects[objects.size() - 1]->data.timing.meter =
                timingSettingsForHitObject[index].meter;
            objects[objects.size() - 1]->data.timing.sampleSet =
                timingSettingsForHitObject[index].sampleSet;
            objects[objects.size() - 1]->data.timing.sampleIndex =
                timingSettingsForHitObject[index].sampleIndex;
            objects[objects.size() - 1]->data.timing.volume =
                timingSettingsForHitObject[index].volume;
            objects[objects.size() - 1]->data.timing.uninherited =
                timingSettingsForHitObject[index].uninherited;
            objects[objects.size() - 1]->data.timing.effects =
                timingSettingsForHitObject[index].effects;
            objects[objects.size() - 1]->data.timing.sliderSpeedOverride =
                timingSettingsForHitObject[index].sliderSpeedOverride;
            objects[objects.size() - 1]->data.index = objects.size() - 1;
            objects[objects.size() - 1]->data.textureReady = false;
            objects[objects.size() - 1]->data.textureLoaded = false;
            objects[objects.size() - 1]->data.timing.renderTicks =
                timingSettingsForHitObject[index].renderTicks;
            /*std::cout << "Time:" << timingSettingsForHitObject[index].time << " Beat:" <<
            objects[objects.size()-1]->data.timing.beatLength << " Meter:" <<
            objects[objects.size()-1]->data.timing.meter << " SV:" <<
            objects[objects.size()-1]->data.timing.sliderSpeedOverride << " SS:" << sliderSpeed << "
            RT:" << objects[objects.size()-1]->data.timing.renderTicks;*/

            objects[objects.size() - 1]->init();
            /*Vector2 templastCords = {objects[objects.size()-1]->data.ex,
            objects[objects.size()-1]->data.ey}; objects[objects.size()-1]->data.ex = lastCords.x;
            objects[objects.size()-1]->data.ey = lastCords.y;
            objects[objects.size()-1]->data.lastTime = lastHitTime;*/
            lastHitTime = objects[objects.size() - 1]->data.time;
            if (objects[objects.size() - 1]->data.type == 2) {
                objects[objects.size() - 1]->data.time +
                    (objects[objects.size() - 1]->data.length / 100) *
                        (objects[objects.size() - 1]->data.timing.beatLength) /
                        (sliderSpeed *
                         objects[objects.size() - 1]->data.timing.sliderSpeedOverride) *
                        objects[objects.size() - 1]->data.slides;
            }
            // lastCords = templastCords;

            // std::thread objectThread(std::bind(&HitObject::init, objects[objects.size()-1]));
            // objectThread.join();
            gameFile.hitObjects.pop_back();
            spawnedHitObjects++;
            for (int amog = 0; amog < index - 1; amog++) {
                timingSettingsForHitObject.erase(timingSettingsForHitObject.begin());
            }
            //}
        } else
            break;
    }
    // update and check collision for every hit circle
    int newSize = objects.size();
    int oldSize = objects.size();
    int susSize = objects.size();
    bool stop = true;
    if (susSize == 0) {
        Global.AutoMouseStartTime = currentTime * 1000.0f;
    }
    for (int i = 0; i < objects.size(); i++) {
        // if((std::abs(currentTime*1000 - objects[i]->data.time) <= gameFile.p50Final)){

        if (IsKeyPressed(SDL_SCANCODE_LEFT)) {
            Global.AutoMouseStartTime = currentTime * 1000.0f;
            Global.AutoMousePositionStart = {320, 240};
        }

        if (i == 0) {
            objects[i]->data.touch = true;
            Global.AutoMousePosition =
                lerp(Global.AutoMousePositionStart, {objects[i]->data.x, objects[i]->data.y},
                     clip((currentTime * 1000.0f - Global.AutoMouseStartTime) /
                              (objects[i]->data.time - Global.AutoMouseStartTime),
                          0, 1));
        }
        if (stop && i == 0 && (Global.Key1P or Global.Key2P)) {
            if (objects[i]->data.type != 2) {
                if (CheckCollisionPointCircle(
                        Global.MousePosition,
                        Vector2{objects[i]->data.x, (float)objects[i]->data.y},
                        circlesize / 2.0f)) {
                    if (std::abs(currentTime * 1000.0f - objects[i]->data.time) >
                        gameFile.p50Final + Global.extraJudgementTime / 2.0f) {
                        objects[i]->data.point = 0;
                        if (clickCombo > 30) {
                            SetSoundVolume(SoundFilesAll.data["combobreak"], 1.0f);
                            PlaySound(SoundFilesAll.data["combobreak"]);
                        }
                        clickCombo = 0;
                        Global.Key1P = false;
                        Global.Key2P = false;
                    } else if (std::abs(currentTime * 1000.0f - objects[i]->data.time) >
                               gameFile.p100Final + Global.extraJudgementTime / 2.0f) {
                        objects[i]->data.point = 1;
                        score +=
                            50 +
                            (50 * (std::max(clickCombo - 1, 0) * difficultyMultiplier * 1) / 25);
                        clickCombo++;
                        Global.errorDiv++;
                        Global.errorLast =
                            (long long)((currentTime * 1000.0f - objects[i]->data.time) * 1000.0f);
                        Global.errorSum += Global.errorLast;
                        Global.Key1P = false;
                        Global.Key2P = false;
                    } else if (std::abs(currentTime * 1000.0f - objects[i]->data.time) >
                               gameFile.p300Final + Global.extraJudgementTime / 2.0f) {
                        objects[i]->data.point = 2;
                        score +=
                            100 +
                            (100 * (std::max(clickCombo - 1, 0) * difficultyMultiplier * 1) / 25);
                        clickCombo++;
                        Global.errorDiv++;
                        Global.errorLast =
                            (long long)((currentTime * 1000.0f - objects[i]->data.time) * 1000.0f);
                        Global.errorSum += Global.errorLast;
                        Global.Key1P = false;
                        Global.Key2P = false;
                    } else {
                        objects[i]->data.point = 3;
                        score +=
                            300 +
                            (300 * (std::max(clickCombo - 1, 0) * difficultyMultiplier * 1) / 25);
                        clickCombo++;
                        Global.errorDiv++;
                        Global.errorLast =
                            (long long)((currentTime * 1000.0f - objects[i]->data.time) * 1000.0f);
                        Global.errorSum += Global.errorLast;
                        Global.Key1P = false;
                        Global.Key2P = false;
                    }
                    int volume = objects[i]->data.volume;
                    if (volume == 0) {
                        objects[i]->data.volume = objects[i]->data.timing.volume;
                        volume = objects[i]->data.volume;
                    }

                    std::vector<std::string> sounds = getAudioFilenames(
                        currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex,
                        defaultSampleSet, objects[i]->data.normalSet, objects[i]->data.additionSet,
                        objects[i]->data.hitSound, objects[i]->data.hindex,
                        objects[i]->data.filename);

                    for (int soundIndex = 0; soundIndex < sounds.size(); soundIndex += 2) {
                        if (SoundFilesAll.data.count(sounds[soundIndex]) == 1 and
                            SoundFilesAll.loaded[sounds[soundIndex]].value) {
                            SetSoundPan(SoundFilesAll.data[sounds[soundIndex]],
                                        1 - clip(objects[i]->data.x / 640.0, 0, 1));
                            SetSoundVolume(SoundFilesAll.data[sounds[soundIndex]],
                                           Global.hitVolume * ((float)volume / 100.0f));
                            PlaySound(SoundFilesAll.data[sounds[soundIndex]]);
                            // std::cout << sounds[0] << " played \n";
                        } else if (SoundFilesAll.data.count(sounds[soundIndex + 1]) == 1 and
                                   SoundFilesAll.loaded[sounds[soundIndex + 1]].value) {
                            SetSoundPan(SoundFilesAll.data[sounds[soundIndex + 1]],
                                        1 - clip(objects[i]->data.x / 640.0, 0, 1));
                            SetSoundVolume(SoundFilesAll.data[sounds[soundIndex + 1]],
                                           Global.hitVolume * ((float)volume / 100.0f));
                            PlaySound(SoundFilesAll.data[sounds[soundIndex + 1]]);
                            // std::cout << sounds[1] << " played \n";
                        }
                    }

                    objects[i]->data.time = currentTime * 1000.0f;
                    destroyHitObject(i);
                    newSize = objects.size();
                    stop = false;
                    if (newSize != oldSize) {
                        i--;
                        oldSize = newSize;
                    }
                } else {
                    objects[i]->data.touch = true;
                    objects[i]->update();
                    newSize = objects.size();
                    if (newSize != oldSize) {
                        i--;
                        oldSize = newSize;
                    }
                }
            } else if (objects[i]->data.type == 2) {
                if (Slider *tempslider = dynamic_cast<Slider *>(objects[i])) {
                    if (CheckCollisionPointCircle(
                            Global.MousePosition,
                            Vector2{objects[i]->data.x, (float)objects[i]->data.y},
                            circlesize / 2.0f) &&
                        currentTime * 1000.0f < tempslider->data.time + gameFile.p50Final) {
                        if (std::abs(currentTime * 1000.0f - tempslider->data.time) >
                            gameFile.p50Final + Global.extraJudgementTime / 2.0f) {
                            tempslider->is_hit_at_first = true;
                            stop = false;
                            tempslider->earlyhit = true;
                            if (clickCombo > 30) {
                                SetSoundVolume(SoundFilesAll.data["combobreak"], 1.0f);
                                PlaySound(SoundFilesAll.data["combobreak"]);
                            }
                            clickCombo = 0;
                            Global.Key1P = false;
                            Global.Key2P = false;
                        } else {
                            tempslider->is_hit_at_first = true;
                            stop = false;
                            clickCombo++;
                            Global.Key1P = false;
                            Global.Key2P = false;
                        }
                        int volume = tempslider->data.volume;
                        if (volume == 0) {
                            tempslider->data.volume = tempslider->data.timing.volume;
                            volume = tempslider->data.volume;
                        }

                        std::vector<std::string> sounds = getAudioFilenames(
                            currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex,
                            defaultSampleSet, objects[i]->data.edgeSets[0].first,
                            objects[i]->data.edgeSets[0].second, objects[i]->data.edgeSounds[0],
                            objects[i]->data.hindex, objects[i]->data.filename);

                        for (int soundIndex = 0; soundIndex < sounds.size(); soundIndex += 2) {
                            if (SoundFilesAll.data.count(sounds[soundIndex]) == 1 and
                                SoundFilesAll.loaded[sounds[soundIndex]].value) {
                                SetSoundPan(SoundFilesAll.data[sounds[soundIndex]],
                                            1 - clip(objects[i]->data.x / 640.0, 0, 1));
                                SetSoundVolume(SoundFilesAll.data[sounds[soundIndex]],
                                               Global.hitVolume * ((float)volume / 100.0f));
                                PlaySound(SoundFilesAll.data[sounds[soundIndex]]);
                                // std::cout << sounds[0] << " played \n";
                            } else if (SoundFilesAll.data.count(sounds[soundIndex + 1]) == 1 and
                                       SoundFilesAll.loaded[sounds[soundIndex + 1]].value) {
                                SetSoundPan(SoundFilesAll.data[sounds[soundIndex + 1]],
                                            1 - clip(objects[i]->data.x / 640.0, 0, 1));
                                SetSoundVolume(SoundFilesAll.data[sounds[soundIndex + 1]],
                                               Global.hitVolume * ((float)volume / 100.0f));
                                PlaySound(SoundFilesAll.data[sounds[soundIndex + 1]]);
                                // std::cout << sounds[1] << " played \n";
                            }
                        }
                    }
                }
                // this cursed else train is nothing to worry about...
                objects[i]->data.index = i;
                objects[i]->update();
                newSize = objects.size();
                if (newSize != oldSize) {
                    i--;
                    oldSize = newSize;
                }
            } else {
                objects[i]->data.index = i;
                objects[i]->update();
                newSize = objects.size();
                if (newSize != oldSize) {
                    i--;
                    oldSize = newSize;
                }
            }
            Global.Key1P = false;
            Global.Key2P = false;
        } else {
            bool debugf = IsKeyDown(SDL_SCANCODE_LEFT);
            if (debugf) {
                // std::cout << "updating object on time: " << objects[i]->data.time << std::endl;

                newSize = objects.size();
                if (newSize != oldSize) {
                    i--;
                    oldSize = newSize;
                } else {
                    if (objects[i]->data.point != 3 &&
                        currentTime * 1000.0f > objects[i]->data.time) {
                        if (objects[i]->data.type != 2) {
                            // Global.MousePosition = {objects[i]->data.x, objects[i]->data.y};
                            objects[i]->data.point = 3;
                            score +=
                                300 +
                                (300 * (std::max(clickCombo - 1, 0) * difficultyMultiplier * 1) /
                                 25);
                            clickCombo++;
                            int volume = objects[i]->data.volume;
                            if (volume == 0) {
                                objects[i]->data.volume = objects[i]->data.timing.volume;
                                volume = objects[i]->data.volume;
                            }

                            std::vector<std::string> sounds = getAudioFilenames(
                                currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex,
                                defaultSampleSet, objects[i]->data.normalSet,
                                objects[i]->data.additionSet, objects[i]->data.hitSound,
                                objects[i]->data.hindex, objects[i]->data.filename);

                            // std::cout << sounds.size() << std::endl;

                            for (int soundIndex = 0; soundIndex < sounds.size(); soundIndex += 2) {
                                if (SoundFilesAll.data.count(sounds[soundIndex]) == 1 and
                                    SoundFilesAll.loaded[sounds[soundIndex]].value) {
                                    SetSoundPan(SoundFilesAll.data[sounds[soundIndex]],
                                                1 - clip(objects[i]->data.x / 640.0, 0, 1));
                                    SetSoundVolume(SoundFilesAll.data[sounds[soundIndex]],
                                                   Global.hitVolume * ((float)volume / 100.0f));
                                    PlaySound(SoundFilesAll.data[sounds[soundIndex]]);
                                    // std::cout << sounds[0] << " played \n";
                                } else if (SoundFilesAll.data.count(sounds[soundIndex + 1]) == 1 and
                                           SoundFilesAll.loaded[sounds[soundIndex + 1]].value) {
                                    SetSoundPan(SoundFilesAll.data[sounds[soundIndex + 1]],
                                                1 - clip(objects[i]->data.x / 640.0, 0, 1));
                                    SetSoundVolume(SoundFilesAll.data[sounds[soundIndex + 1]],
                                                   Global.hitVolume * ((float)volume / 100.0f));
                                    PlaySound(SoundFilesAll.data[sounds[soundIndex + 1]]);
                                    // std::cout << sounds[1] << " played \n";
                                }
                            }

                            // std::cout << 3.5*easeInOutCubic((1-(currentTime*1000.0f -
                            // objects[i]->data.time + gameFile.preempt)/gameFile.preempt))+1 <<
                            // std::endl;
                            objects[i]->data.time = currentTime * 1000.0f;

                            Global.AutoMousePositionStart = {objects[i]->data.x,
                                                             objects[i]->data.y};
                            Global.AutoMouseStartTime = currentTime * 1000.0f;

                            destroyHitObject(i);
                            newSize = objects.size();
                            stop = false;
                            if (newSize != oldSize) {
                                i--;
                                oldSize = newSize;
                            }
                        } else if (objects[i]->data.type == 2) {
                            Slider *tempslider = dynamic_cast<Slider *>(objects[i]);
                            tempslider->is_hit_at_first = true;
                            objects[i]->data.point = 3;
                            stop = false;
                            clickCombo++;
                            int volume = tempslider->data.volume;
                            if (volume == 0) {
                                tempslider->data.volume = tempslider->data.timing.volume;
                                volume = tempslider->data.volume;
                            }

                            std::vector<std::string> sounds = getAudioFilenames(
                                currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex,
                                defaultSampleSet, objects[i]->data.edgeSets[0].first,
                                objects[i]->data.edgeSets[0].second, objects[i]->data.edgeSounds[0],
                                objects[i]->data.hindex, objects[i]->data.filename);

                            for (int soundIndex = 0; soundIndex < sounds.size(); soundIndex += 2) {
                                if (SoundFilesAll.data.count(sounds[soundIndex]) == 1 and
                                    SoundFilesAll.loaded[sounds[soundIndex]].value) {
                                    SetSoundPan(SoundFilesAll.data[sounds[soundIndex]],
                                                1 - clip(objects[i]->data.x / 640.0, 0, 1));
                                    SetSoundVolume(SoundFilesAll.data[sounds[soundIndex]],
                                                   Global.hitVolume * ((float)volume / 100.0f));
                                    PlaySound(SoundFilesAll.data[sounds[soundIndex]]);
                                    // std::cout << sounds[0] << " played \n";
                                } else if (SoundFilesAll.data.count(sounds[soundIndex + 1]) == 1 and
                                           SoundFilesAll.loaded[sounds[soundIndex + 1]].value) {
                                    SetSoundPan(SoundFilesAll.data[sounds[soundIndex + 1]],
                                                1 - clip(objects[i]->data.x / 640.0, 0, 1));
                                    SetSoundVolume(SoundFilesAll.data[sounds[soundIndex + 1]],
                                                   Global.hitVolume * ((float)volume / 100.0f));
                                    PlaySound(SoundFilesAll.data[sounds[soundIndex + 1]]);
                                    // std::cout << sounds[1] << " played \n";
                                }
                            }

                            objects[i]->data.index = i;
                            objects[i]->update();
                            newSize = objects.size();
                            if (newSize != oldSize) {
                                i--;
                                oldSize = newSize;
                            }
                        }
                    } else {
                        if (objects[i]->data.type == 2) {
                            objects[i]->data.index = i;
                            objects[i]->update();
                            newSize = objects.size();
                            if (newSize != oldSize) {
                                i--;
                                oldSize = newSize;
                            }
                        }
                    }
                }

                newSize = objects.size();
                if (newSize != oldSize) {
                    i--;
                    oldSize = newSize;
                }
            } else {
                objects[i]->data.index = i;
                objects[i]->update();
                newSize = objects.size();
                if (newSize != oldSize) {
                    i--;
                    oldSize = newSize;
                }
            }
        }
    }

    int deadoldsize = dead_objects.size();
    int deadnewsize = dead_objects.size();

    for (int i = 0; i < dead_objects.size(); i++) {
        dead_objects[i]->data.index = i;
        dead_objects[i]->dead_update();
        if (dead_objects[i]->data.expired == true) {
            destroyDeadHitObject(i);
            // std::cout << "deleted an object\n";
        }
        deadnewsize = dead_objects.size();
        if (deadnewsize != deadoldsize) {
            i--;
            deadoldsize = deadnewsize;
        }
    }

    for (int i = followLines.size() - 1; i >= 0; i--) {
        if (followLines[i].startTime > currentTime * 1000.0f) {
            break;
        }
        followLines[i].update();
    }

    for (int i = followLines.size() - 1; i >= 0; i--) {
        if (!followLines[i].shouldDelete) {
            break;
        }
        followLines.pop_back();
    }
}

// main rendering loop
void GameManager::render() {
    if (currentBackgroundTexture.length() > 0 &&
        backgroundTextures.loaded[currentBackgroundTexture].value) {
        // std::cout << currentBackgroundTexture << std::endl;
        Global.NeedForBackgroundClear = false;
        DrawTextureCenter(
            backgroundTextures.data[currentBackgroundTexture], 320, 240,
            (double)std::max((double)GetScreenWidth() /
                                 (double)backgroundTextures.data[currentBackgroundTexture].width,
                             (double)GetScreenHeight() /
                                 (double)backgroundTextures.data[currentBackgroundTexture].height) /
                (double)Global.Scale,
            WHITE);
        // DrawRectangle(-5, -5, GetScreenWidth() + 10, GetScreenHeight() + 10, Fade(BLUE, 1.0f));
    }

    for (int i = followLines.size() - 1; i >= 0; i--) {
        if (followLines[i].startTime > currentTime * 1000.0f) {
            break;
        }
        followLines[i].render();
    }

    for (int i = objects.size() - 1; i >= 0; i--) {
        ////Global.mutex.lock();
        /*if(!objects[i]->data.startingACombo){
                float clampedFade = (currentTime*1000.0f - objects[i]->data.lastTime) /
        (objects[i]->data.time - objects[i]->data.lastTime);
                DrawLineEx(ScaleCords(lerp({objects[i]->data.ex, objects[i]->data.ey}
        ,{objects[i]->data.x, objects[i]->data.y}, clip(clampedFade, 0.0f, 1.0f))),
                ScaleCords(lerp({objects[i]->data.ex, objects[i]->data.ey} ,{objects[i]->data.x,
        objects[i]->data.y}, clip(clampedFade + 0.5f, 0.0f, 1.0f))), Scale(3), Fade(WHITE, 0.7f));
        }*/

        objects[i]->render();

        ////Global.mutex.unlock();
    }
    for (int i = dead_objects.size() - 1; i >= 0; i--) {
        ////Global.mutex.lock();
        dead_objects[i]->dead_render();
        ////Global.mutex.unlock();
    }
    DrawCNumbersCenter(score, 320, 10, 0.4f, WHITE);
    DrawCNumbersLeft(clickCombo, 15, 460, 0.6f, WHITE);

    if (spawnedHitObjects == 0 &&
        gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime * 1000.0f) {
        ////Global.mutex.lock();
        DrawTextEx(Global.DefaultFont,
                   TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that this can affect the "
                              "offset\nbecause of how the raylib sounds system works)"),
                   {ScaleCordX(5), ScaleCordY(420)}, Scale(15), Scale(1), WHITE);
        ////Global.mutex.unlock();
    }
    // render the points and the combo
}

void GameManager::run() {
    // start playing the music and set the volume, it gets quite loud
    double Time = 0;
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    // ms = getTimer() / 1000.0;

    if (Global.startTime < 0) {
        Global.startTime += Global.FrameTime;
        Time = Global.startTime;
        // std::cout << Time << std::endl;
    }
    if (Global.startTime >= 0 and startMusic) {
        std::cout << "trying to start music" << std::endl;
        PlayMusicStream(backgroundMusic);
        // Global.volume = 1.0f;
        std::cout << Global.volume << std::endl;
        SetMusicVolume(backgroundMusic, Global.volume);  // Global.volume
        SeekMusicStream(backgroundMusic, 0.0f);
        UpdateMusicStream(backgroundMusic);
        initTimer();
        std::cout << "started music" << std::endl;
        std::cout << "first update" << std::endl;
        Global.CurrentInterpolatedTime = 0;
        Global.LastOsuTime = 0;
        TimeLast = ms;
        startMusic = false;
        Global.startTime2 = ms;
        double Time = (double)GetMusicTimePlayed(backgroundMusic) * 1000.0;
        double amog = getTimer();
        std::cout << "Extra Time in ms " << Global.extraJudgementTime << std::endl;
        std::cout << "Time:" << Time << std::endl;
        Global.avgSum = 0;
        Global.avgNum = 0;
        Global.avgTime = 0;

        Global.LastFrameTime = getTimer();
    }
    if (true) {
        if (Global.volumeChanged) {
            SetMusicVolume(backgroundMusic, Global.volume);
            Global.volumeChanged = false;
        }
        UpdateMusicStream(backgroundMusic);
        if (spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time >
                                          6000 + currentTime * 1000.0f) {
            // DrawTextEx(Global.DefaultFont, TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that
            // this can affect the offset\nbecause of how the raylib sounds system works)"),
            // {ScaleCordX(5), ScaleCordY(420)}, Scale(15), Scale(1), WHITE);
            if (IsKeyPressed(SDL_SCANCODE_S)) {
                SeekMusicStream(
                    backgroundMusic,
                    (gameFile.hitObjects[gameFile.hitObjects.size() - 1].time - 3000.0f) / 1000.0f);
            }
        }
        if (GetMusicTimeLength(backgroundMusic) - GetMusicTimePlayed(backgroundMusic) < 0.05f)
            stop = true;
        if (stop && Global.curTime2 < 1.0f) {
            StopMusicStream(backgroundMusic);
        }

        if (IsMusicStreamPlaying(backgroundMusic)) {
            Time = (double)GetMusicTimePlayed(backgroundMusic) * 1000.0;
            if (!AreSame(TimerLast, Time)) {
                // Global.extraJudgementTime = std::abs((Time - TimerLast) / 1.5f);
                TimerLast = (double)GetMusicTimePlayed(backgroundMusic) * 1000.0;
                TimeLast = ms;
            } else {
                Time += ms - TimeLast;
            }
        } else {
            TimeLast = ms;
        }

        Global.curTime = Time;
        double LastInterpolatedTime = Global.currentOsuTime;

        bool IsInterpolating;

        if (IsMusicStreamPlaying(backgroundMusic)) {
            if (GetMusicTimePlayed(backgroundMusic) * 1000.0 != 0) IsInterpolating = true;
            double ElapsedTime = getTimer() - Global.LastOsuTime;
            Global.LastOsuTime = getTimer();
            Global.CurrentInterpolatedTime += ElapsedTime;
            if (!IsInterpolating || std::abs(GetMusicTimePlayed(backgroundMusic) * 1000.0 -
                                             Global.CurrentInterpolatedTime) > 8) {
                Global.CurrentInterpolatedTime =
                    ElapsedTime < 0 ? GetMusicTimePlayed(backgroundMusic) * 1000.0
                                    : std::max(LastInterpolatedTime,
                                               GetMusicTimePlayed(backgroundMusic) * 1000.0);
                IsInterpolating = false;
                std::cout << "failed interpolation at time " << Global.CurrentInterpolatedTime
                          << "\n";
            } else {
                Global.CurrentInterpolatedTime += (GetMusicTimePlayed(backgroundMusic) * 1000.0 -
                                                   Global.CurrentInterpolatedTime) /
                                                  5;
                Global.CurrentInterpolatedTime =
                    std::max(LastInterpolatedTime, Global.CurrentInterpolatedTime);
            }
        }

        Global.currentOsuTime = IsMusicStreamPlaying(backgroundMusic)
                                    ? Global.CurrentInterpolatedTime
                                    : GetMusicTimePlayed(backgroundMusic);

        currentTime = (double)Time / 1000.0;
        if (IsMusicStreamPlaying(backgroundMusic)) {
            currentTime = (Global.currentOsuTime + Global.offsetTime) / 1000.0;
        }

        // currentTime -= 8/1000.0f;
        // currentTime *= 2;
        GameManager::update();
        // std::cout << "called update at time " << Global.currentOsuTime << "\n";
        // currentTime += 8/1000.0f;
    }
}

std::pair<Vector2, int> get2PerfectCircle(Vector2 &p1, Vector2 &p2, Vector2 &p3) {
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int x3 = p3.x;
    int y3 = p3.y;
    int a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
    int b = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) +
            (x3 * x3 + y3 * y3) * (y2 - y1);
    int c = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) +
            (x3 * x3 + y3 * y3) * (x1 - x2);
    float x = (float)-b / (2.0f * (float)a);
    float y = (float)-c / (2.0f * (float)a);
    return std::make_pair(Vector2{x, y}, sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1)));
}

Vector2 get2BezierPoint(std::vector<Vector2> &points, int numPoints, float t) {
    Vector2 *tmp = new Vector2[numPoints];
    for (size_t i = 0; i < points.size(); i++) {
        tmp[i] = points[i];
    }
    int i = numPoints - 1;
    while (i > 0) {
        for (int k = 0; k < i; k++)
            tmp[k] = Vector2{tmp[k].x + t * (tmp[k + 1].x - tmp[k].x),
                             tmp[k].y + t * (tmp[k + 1].y - tmp[k].y)};
        i--;
    }
    Vector2 answer = tmp[0];
    delete[] tmp;
    return answer;
}

// load the beatmap
void GameManager::loadDefaultSkin(std::string filename) {
    currentComboIndex = 0;
    std::vector<std::string> files;
    files.clear();
    Global.Path = "resources/default_skin/";
    files = ls(".png");

    std::sort(files.begin(), files.end(), [](const std::string &first, const std::string &second) {
        return first.size() < second.size();
    });
    std::reverse(files.begin(), files.end());

    for (int i = 0; i < files.size(); i++) {
        if (IsFileExtension(files[i].c_str(), ".png")) {
            if (files[i].rfind("bubbles.png", 0) == 0)
                hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles.png", 0) == 0)
                selectCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles", 0) == 0)
                hitCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("approachcircle", 0) == 0)
                approachCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit300k", 0) == 0)
                ;
            else if (files[i].rfind("hit300", 0) == 0)
                hit300 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit100k", 0) == 0)
                ;
            else if (files[i].rfind("hit100", 0) == 0)
                hit100 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit50k", 0) == 0)
                ;
            else if (files[i].rfind("hit50", 0) == 0)
                hit50 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit0", 0) == 0)
                hit0 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderscorepoint", 0) == 0)
                sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderfollowcircle", 0) == 0)
                sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles", 0) == 0)
                sliderb = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("reversearrow", 0) == 0)
                reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-circle", 0) == 0) {
                spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
                renderSpinnerCircle = true;
            } else if (files[i].rfind("spinner-metre", 0) == 0) {
                spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
                renderSpinnerMetre = true;
            } else if (files[i].rfind("spinner-bottom", 0) == 0)
                spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-top", 0) == 0)
                spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-approachcircle", 0) == 0)
                spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
            else {
                for (int j = 0; j < 10; j++) {
                    if (files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0) {
                        numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
                    }
                }
            }
        }
    }
}

void GameManager::loadDefaultSound(std::string filename) {}

void GameManager::loadGameSkin(std::string filename) {
    temprenderSpinnerCircle = false;
    temprenderSpinnerMetre = false;
    temprenderSpinnerBack = false;

    std::vector<std::string> files;
    files.clear();
    Global.Path = "resources/skin/";
    files = ls(".png");

    std::sort(files.begin(), files.end(), [](const std::string &first, const std::string &second) {
        return first.size() < second.size();
    });
    std::reverse(files.begin(), files.end());

    for (int i = 0; i < files.size(); i++) {
        if (IsFileExtension(files[i].c_str(), ".png")) {
            if (files[i].rfind("bubbles.png", 0) == 0)
                hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles.png", 0) == 0)
                selectCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles", 0) == 0)
                hitCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("approachcircle", 0) == 0)
                approachCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit300k", 0) == 0)
                ;
            else if (files[i].rfind("hit300", 0) == 0)
                hit300 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit100k", 0) == 0)
                ;
            else if (files[i].rfind("hit100", 0) == 0)
                hit100 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit50k", 0) == 0)
                ;
            else if (files[i].rfind("hit50", 0) == 0)
                hit50 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit0", 0) == 0)
                hit0 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderscorepoint", 0) == 0)
                sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderfollowcircle", 0) == 0)
                sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles", 0) == 0)
                sliderb = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("reversearrow", 0) == 0)
                reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-circle", 0) == 0) {
                spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
                temprenderSpinnerCircle = true;
            } else if (files[i].rfind("spinner-metre", 0) == 0) {
                spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
                temprenderSpinnerMetre = true;
            } else if (files[i].rfind("spinner-background", 0) == 0) {
                spinnerBack = LoadTexture((Global.Path + files[i]).c_str());
                temprenderSpinnerBack = true;
            } else if (files[i].rfind("spinner-bottom", 0) == 0)
                spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-top", 0) == 0)
                spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-approachcircle", 0) == 0)
                spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
            else {
                for (int j = 0; j < 10; j++) {
                    if (files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0) {
                        numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
                    }
                }
            }
        }
    }
}

void GameManager::loadGameSound(std::string filename) {}

void GameManager::loadBeatmapSkin(std::string filename) {
    std::vector<std::string> files;
    files.clear();
    Global.Path = lastPath + '/';

    files = ls(".png");

    std::sort(files.begin(), files.end(), [](const std::string &first, const std::string &second) {
        return first.size() < second.size();
    });
    std::reverse(files.begin(), files.end());
    temprenderSpinnerCircle = false;
    temprenderSpinnerMetre = false;
    temprenderSpinnerBack = false;
    for (int i = 0; i < files.size(); i++) {
        if (IsFileExtension(files[i].c_str(), ".png")) {
            if (files[i].rfind("bubbles.png", 0) == 0)
                hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles.png", 0) == 0)
                selectCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("bubbles", 0) == 0)
                hitCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("approachcircle", 0) == 0)
                approachCircle = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit300k", 0) == 0)
                ;
            else if (files[i].rfind("hit300", 0) == 0)
                hit300 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit100k", 0) == 0)
                ;
            else if (files[i].rfind("hit100", 0) == 0)
                hit100 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit50k", 0) == 0)
                ;
            else if (files[i].rfind("hit50", 0) == 0)
                hit50 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("hit0", 0) == 0)
                hit0 = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderscorepoint", 0) == 0)
                sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderfollowcircle", 0) == 0)
                sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("sliderb0", 0) == 0)
                sliderb = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("reversearrow", 0) == 0)
                reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-circle", 0) == 0) {
                spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
                renderSpinnerCircle = true;
            } else if (files[i].rfind("spinner-metre", 0) == 0) {
                spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
                renderSpinnerMetre = true;
            } else if (files[i].rfind("spinner-background", 0) == 0) {
                spinnerBack = LoadTexture((Global.Path + files[i]).c_str());
                temprenderSpinnerBack = true;
            } else if (files[i].rfind("spinner-bottom", 0) == 0)
                spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-top", 0) == 0)
                spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
            else if (files[i].rfind("spinner-approachcircle", 0) == 0)
                spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
            else {
                for (int j = 0; j < 10; j++) {
                    if (files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0) {
                        numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
                    }
                }
            }
        }
    }
}

void GameManager::loadBeatmapSound(std::string filename) {}

void GameManager::loadGame(std::string filename) {
    // create a parser and parse the file
    currentBackgroundTexture = "";

    // misc variables
    spawnedHitObjects = 0;
    Parser parser = Parser();
    Global.loadingState = 5;
    gameFile.configGeneral["SampleSet"] = "Normal";
    std::cout << "Parsing game!" << std::endl;
    gameFile = parser.parse(filename);
    std::cout << "Found " << gameFile.hitObjects.size() << " HitObjects and "
              << gameFile.timingPoints.size() << " Timing Points!" << std::endl;
    Global.loadingState = 1;
    Global.numberLines = gameFile.hitObjects.size();
    Global.parsedLines = 0;

    // reverse the hitobject array because we need it reversed for it to make sense (and make it
    // faster because pop_back)
    lastTimingLoc = gameFile.timingPoints.size() - 1;
    lastCurrentTiming = gameFile.timingPoints.size() - 1;

    GamePathWithSlash = Global.Path + '/';

    lastPath = Global.Path;

    //---------------------------------------------------------------------------------------------------------------------------------------------------------
    //------------------------------------------------------------------|LOADING
    // BACKGROUNDS|------------------------------------------------------------------
    //---------------------------------------------------------------------------------------------------------------------------------------------------------

    // clear all the backgrounds we have
    backgroundTextures.data.clear();
    backgroundTextures.pos.clear();
    backgroundTextures.loaded.clear();

    // check for png backgrounds
    std::vector<std::string> files;
    files.clear();
    Global.Path = lastPath + '/';
    files = ls(".png");

    // precalculate all the sliders and check how long we need to wait for it
    double start = getTimer();
    for (int i = 0; i < gameFile.hitObjects.size(); i++) {
        if (gameFile.hitObjects[i].type == 2) {
            std::vector<Vector2> edgePoints;
            edgePoints.push_back(
                Vector2{(float)gameFile.hitObjects[i].x, (float)gameFile.hitObjects[i].y});
            float resolution = gameFile.hitObjects[i].length;
            float currentResolution = 0;
            float lengthScale, totalLength = 0;

            for (size_t j = 0; j < gameFile.hitObjects[i].curvePoints.size(); j++)
                edgePoints.push_back(Vector2{(float)gameFile.hitObjects[i].curvePoints[j].first,
                                             (float)gameFile.hitObjects[i].curvePoints[j].second});

            if (edgePoints.size() == 2 and gameFile.hitObjects[i].curveType == 'B') {
                gameFile.hitObjects[i].curveType == 'L';
            }

        redoCalc:

            if (gameFile.hitObjects[i].curveType == 'L') {
                std::vector<float> lineLengths;
                // std::cout << "will calculate linear slider id " << i << " at time " <<
                // gameFile.hitObjects[i].time << std::endl;
                for (size_t j = 0; j < edgePoints.size() - 1; j++)
                    lineLengths.push_back(
                        std::sqrt(std::pow(std::abs(edgePoints[j].x - edgePoints[j + 1].x), 2) +
                                  std::pow(std::abs(edgePoints[j].y - edgePoints[j + 1].y), 2)));
                for (size_t j = 0; j < lineLengths.size(); j++) totalLength += lineLengths[j];
                float angle =
                    atan2(
                        edgePoints[edgePoints.size() - 1].y - edgePoints[edgePoints.size() - 2].y,
                        edgePoints[edgePoints.size() - 1].x - edgePoints[edgePoints.size() - 2].x) *
                    180 / 3.14159265;
                float hipotenus = gameFile.hitObjects[i].length - totalLength;
                float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
                float ydiff = sqrt(std::abs(hipotenus * hipotenus - xdiff * xdiff));
                int ything = 1;
                if (angle < 0.0f) {
                    ything = -1;
                } else if (angle == 0.0f) {
                    ything = 0;
                }

                Vector2 extraPosition = {
                    edgePoints[edgePoints.size() - 1].x + xdiff,
                    edgePoints[edgePoints.size() - 1].y - ydiff * (float)ything};

                gameFile.hitObjects[i].totalLength = totalLength;
                gameFile.hitObjects[i].lengths = lineLengths;
                gameFile.hitObjects[i].extraPos = extraPosition;
            }
            if (gameFile.hitObjects[i].curveType == 'P') {
                if ((edgePoints[0].x == edgePoints[2].x and edgePoints[0].y == edgePoints[2].y) or
                    (edgePoints[1].x == edgePoints[2].x and edgePoints[1].y == edgePoints[2].y) or
                    (edgePoints[0].x == edgePoints[1].x and edgePoints[0].y == edgePoints[1].y)) {
                    gameFile.hitObjects[i].curveType = 'L';
                    goto redoCalc;
                } else {
                    std::pair<Vector2, float> circleData =
                        get2PerfectCircle(edgePoints[0], edgePoints[1], edgePoints[2]);
                    float inf = std::numeric_limits<float>::infinity();
                    if (circleData.first.x == -inf or circleData.first.x == inf or
                        circleData.first.y == -inf or circleData.first.y == inf) {
                        std::vector<float> lineLengths;
                        // std::cout << "will calculate linear slider id " << i << " at time " <<
                        // gameFile.hitObjects[i].time << std::endl;
                        for (size_t j = 0; j < edgePoints.size() - 1; j++)
                            lineLengths.push_back(std::sqrt(
                                std::pow(std::abs(edgePoints[j].x - edgePoints[j + 1].x), 2) +
                                std::pow(std::abs(edgePoints[j].y - edgePoints[j + 1].y), 2)));
                        for (size_t j = 0; j < lineLengths.size(); j++)
                            totalLength += lineLengths[j];
                        float angle = atan2(edgePoints[edgePoints.size() - 1].y -
                                                edgePoints[edgePoints.size() - 2].y,
                                            edgePoints[edgePoints.size() - 1].x -
                                                edgePoints[edgePoints.size() - 2].x) *
                                      180 / 3.14159265;
                        float hipotenus = gameFile.hitObjects[i].length - totalLength;
                        float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
                        float ydiff = sqrt(std::abs(hipotenus * hipotenus - xdiff * xdiff));
                        int ything = 1;
                        if (angle < 0.0f) {
                            ything = -1;
                        } else if (angle == 0.0f) {
                            ything = 0;
                        }

                        Vector2 extraPosition = {
                            edgePoints[edgePoints.size() - 1].x + xdiff,
                            edgePoints[edgePoints.size() - 1].y - ydiff * (float)ything};

                        gameFile.hitObjects[i].curveType = 'L';

                        gameFile.hitObjects[i].totalLength = totalLength;
                        gameFile.hitObjects[i].lengths = lineLengths;
                        gameFile.hitObjects[i].extraPos = extraPosition;
                    } else {
                        // std::cout << "will NOT calculate perfect circle slider id " << i << " at
                        // time " << gameFile.hitObjects[i].time << std::endl;
                    }
                }
            }
            if (gameFile.hitObjects[i].curveType == 'B') {
                bool old = true;
                if (old) {
                    // std::cout << "will calculate bezier slider id " << i << " at time " <<
                    // gameFile.hitObjects[i].time << std::endl;
                    Vector2 edges[edgePoints.size()];
                    for (size_t j = 0; j < edgePoints.size(); j++) edges[j] = edgePoints[j];
                    std::vector<Vector2> tempEdges;
                    std::vector<Vector2> tempRender;
                    std::vector<float> curveLengths;
                    double totalCalculatedLength = 0;
                    int curves = 0;
                    for (size_t j = 0; j < edgePoints.size(); j++) {
                        if (j == edgePoints.size() - 1 ||
                            (edgePoints[j].x == edgePoints[j + 1].x &&
                             edgePoints[j].y == edgePoints[j + 1].y)) {
                            curves++;
                        }
                    }
                    for (size_t k = 0; k < edgePoints.size(); k++) {
                        tempEdges.push_back(edgePoints[k]);
                        if (k == edgePoints.size() - 1 ||
                            (edgePoints[k].x == edgePoints[k + 1].x &&
                             edgePoints[k].y == edgePoints[k + 1].y)) {
                            currentResolution = 0;
                            int num = tempEdges.size();
                            num = std::max((int)(gameFile.hitObjects[i].length / curves), 50);
                            int m = 0;
                            float tempLength = 0;
                            Vector2 lasttmp;
                            while (true) {
                                if (currentResolution > num) break;
                                float j = (float)currentResolution / (float)num;

                                Vector2 tmp = get2BezierPoint(tempEdges, tempEdges.size(), j);
                                if (m >= 1)
                                    tempLength += std::sqrt(std::pow(lasttmp.x - tmp.x, 2) +
                                                            std::pow(lasttmp.y - tmp.y, 2));
                                lasttmp = tmp;
                                currentResolution++;
                                m++;
                            }
                            curveLengths.push_back(tempLength);
                            // sometimes + 1 is better?????
                            totalCalculatedLength += tempLength;
                            tempEdges.clear();
                        }
                    }

                    gameFile.hitObjects[i].totalLength = totalCalculatedLength;
                    gameFile.hitObjects[i].lengths = curveLengths;
                    curveLengths.clear();
                    tempEdges.clear();
                    tempRender.clear();
                } else {
                }
            }
        }
    }
    std::cout << "Sliders precalculated in " << getTimer() - start << "ms" << std::endl;
    Global.loadingState = 2;

    // calculate all the variables for the game (these are mathematically correct but they feel
    // weird?)
    if (std::stof(gameFile.configDifficulty["ApproachRate"]) < 5.0f) {
        gameFile.preempt =
            1200.0f + 600.0f * (5.0f - std::stof(gameFile.configDifficulty["ApproachRate"])) / 5.0f;
        gameFile.fade_in =
            800.0f + 400.0f * (5.0f - std::stof(gameFile.configDifficulty["ApproachRate"])) / 5.0f;
    } else if (std::stof(gameFile.configDifficulty["ApproachRate"]) > 5.0f) {
        gameFile.preempt =
            1200.0f - 750.0f * (std::stof(gameFile.configDifficulty["ApproachRate"]) - 5.0f) / 5.0f;
        gameFile.fade_in =
            800.0f - 500.0f * (std::stof(gameFile.configDifficulty["ApproachRate"]) - 5.0f) / 5.0f;
    } else {
        gameFile.preempt = 1200.0f;
        gameFile.fade_in = 800.0f;
    }
    // TODO: Spinners are still in their initial state, this is probably pretty wrong
    float od = std::stoi(gameFile.configDifficulty["OverallDifficulty"]);
    if (od < 5) {
        spinsPerSecond = 5.0f - 2.0f * (5.0f - od) / 5.0f;
    } else if (od > 5) {
        spinsPerSecond = 5.0f - 2.0f * (od - 5.0f) / 5.0f;
    } else {
        spinsPerSecond = 5.0f;
    }

    // calculate the time windows for 300-100-50 points
    gameFile.p300Final = gameFile.p300 - std::stof(gameFile.configDifficulty["OverallDifficulty"]) *
                                             gameFile.p300Change;
    gameFile.p100Final = gameFile.p100 - std::stof(gameFile.configDifficulty["OverallDifficulty"]) *
                                             gameFile.p100Change;
    gameFile.p50Final = gameFile.p50 - std::stof(gameFile.configDifficulty["OverallDifficulty"]) *
                                           gameFile.p50Change;

    // debug lines
    std::cout << gameFile.p300Final << " " << gameFile.p100Final << " " << gameFile.p50Final << " "
              << gameFile.preempt << std::endl;
    std::cout << std::stof(gameFile.configDifficulty["OverallDifficulty"]) << " "
              << gameFile.configDifficulty["OverallDifficulty"] << std::endl;
    std::cout << std::stof(gameFile.configDifficulty["ApproachRate"]) << " "
              << gameFile.configDifficulty["ApproachRate"] << std::endl;

    // debug, just say what the name of the music file is and load it
    // std::cout << (Global.Path + '/' + gameFile.configGeneral["AudioFilename"]) << std::endl;
    // backgroundMusic = LoadMusicStream((Global.Path + '/' +
    // gameFile.configGeneral["AudioFilename"]).c_str());

    // get the file size of the music file and allocate memory for it
    FILE *music =
        fopen((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str(), "rb");
    fseek(music, 0, SEEK_END);
    musicSize = ftell(music);
    fseek(music, 0, SEEK_SET); /* same as rewind(f); */
    musicData = (char *)malloc(musicSize + 1);
    fread(musicData, musicSize, 1, music);
    fclose(music);
    musicData[musicSize] = 0;
    // load the music as a raylib music file
    backgroundMusic = LoadMusicStreamFromMemory(
        GetFileExtension((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str()),
        (const unsigned char *)musicData, musicSize);

    Global.loadingState = 6;

    // reset the score and the combo
    score = 0;
    clickCombo = 0;

    // TODO: these are not used right now, USE THEM
    float hpdrainrate = std::stof(gameFile.configDifficulty["HPDrainRate"]);

    // circle size calculations
    circlesize = 54.4f - (4.48f * std::stof(gameFile.configDifficulty["CircleSize"]));
    slidertickrate = std::stof(gameFile.configDifficulty["SliderTickRate"]);
    circlesize *= 2.0f;

    // more difficulty stuff, may also be wrong
    float overalldifficulty = std::stof(gameFile.configDifficulty["OverallDifficulty"]);
    difficultyMultiplier =
        ((hpdrainrate + circlesize + overalldifficulty +
          clip((float)gameFile.hitObjects.size() / GetMusicTimeLength(backgroundMusic) * 8.f, 0.f,
               16.f)) /
         38.f * 5.f);
    if (gameFile.configDifficulty.find("SliderMultiplier") != gameFile.configDifficulty.end())
        sliderSpeed = std::stof(gameFile.configDifficulty["SliderMultiplier"]);

    Global.GameTextures = 2;
    /*GameManager::loadDefaultSkin(filename); // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
    GameManager::loadGameSkin(filename); // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
    if(!IsKeyDown(KEY_S)){
            GameManager::loadBeatmapSkin(filename); // LOADING THE BEATMAP SKIN USING A SEPERATE
    FUNCTION
    }*/

    // spinner debug lines
    if (temprenderSpinnerCircle == true) {
        renderSpinnerCircle = true;
        std::cout << "================================== RENDERING THE SPINNER CIRCLE "
                     "==================================\n";
    } else {
        renderSpinnerCircle = false;
    }
    if (temprenderSpinnerMetre == true) {
        renderSpinnerMetre = true;
        std::cout << "=================================== RENDERING THE SPINNER METRE "
                     "==================================\n";
    } else {
        renderSpinnerMetre = false;
    }
    if (temprenderSpinnerBack == true) {
        renderSpinnerBack = true;
        std::cout << "================================ RENDERING THE SPINNER BACKGROUND "
                     "================================\n";
    } else {
        renderSpinnerBack = false;
    }

    // load all of the hitsounds into memory
    loadGameSounds();

    // redundant but meh...
    files.clear();
    Global.Path = "resources/skin/";
    files = ls(".wav");

    // followpoint precalculations/creation. These values are wrong.
    // TODO: fix this bs
    std::reverse(gameFile.timingPoints.begin(), gameFile.timingPoints.end());
    std::reverse(gameFile.events.begin(), gameFile.events.end());

    // create temporary timing points for the follow points (useful for sliders)
    timingSettings tempTiming;
    std::vector<timingSettings> times;
    double preCalcLength;
    for (int i = gameFile.timingPoints.size() - 1; i >= 0; i--) {
        tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
        tempTiming.sliderSpeedOverride = 1;
        tempTiming.time = gameFile.timingPoints[i].time;
        double tempBeatLength;
        tempBeatLength = gameFile.timingPoints[i].beatLength;
        if (tempBeatLength >= 0) {
            tempTiming.beatLength = tempBeatLength;
            preCalcLength = tempBeatLength;
            tempTiming.sliderSpeedOverride = 1;
        }
        if (tempBeatLength < 0) {
            tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
            tempTiming.beatLength = preCalcLength;
        }
        tempTiming.meter = gameFile.timingPoints[i].meter;
        tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
        tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
        tempTiming.volume = gameFile.timingPoints[i].volume;
        tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
        tempTiming.effects = gameFile.timingPoints[i].effects;
        // std::cout << "push timing point at: " << tempTiming.time << "\n";
        times.push_back(tempTiming);
    }

    int index = 0;

    // fade in time for followpoints, this part i dont really understand since its not really
    // documented

    float followPointFadeTime = gameFile.preempt - gameFile.fade_in;
    followLines.clear();
    for (int i = 1; i < gameFile.hitObjects.size(); i++) {
        // float templength = (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed *
        // data.timing.sliderSpeedOverride) * data.slides; //slider length
        if (gameFile.hitObjects[i].startingACombo == false and
            gameFile.hitObjects[i - 1].type != 3) {
            FollowPoint tempPoint;
            tempPoint.endTime = gameFile.hitObjects[i].time;
            tempPoint.endTime2 = gameFile.hitObjects[i].time + followPointFadeTime;
            tempPoint.endX = gameFile.hitObjects[i].x;
            tempPoint.endY = gameFile.hitObjects[i].y;

            if (gameFile.hitObjects[i - 1].type == 2) {
                /*if(data.slides % 2 == 0){
                        data.ex = data.x;
                        data.ey = data.y;
                }
                else{
                        data.ex = renderPoints[renderPoints.size() - 1].x;
                        data.ey = renderPoints[renderPoints.size() - 1].y;
                }*/
                HitObjectData tempData = gameFile.hitObjects[i - 1];
                while (true) {
                    if (index + 1 > times.size() - 1) break;
                    if (times[index + 1].time > tempData.time) break;
                    index++;
                }

                //--------------------------------------------- STANDART SLIDER PROCEDURE
                //---------------------------------------------

                tempData.timing.beatLength = times[index + 1].beatLength;
                tempData.timing.sliderSpeedOverride = times[index + 1].sliderSpeedOverride;
                std::vector<int> output = sliderPreInit(tempData);

                tempPoint.startTime = output[2] - followPointFadeTime;
                tempPoint.startTime2 = output[2];
                tempPoint.startX = output[0];
                tempPoint.startY = output[1];

                // std::cout << "done calculation of follow line starting from slider at time: " <<
                // tempData.time << "\n";
                //--------------------------------------------- STANDART SLIDER PROCEDURE
                //---------------------------------------------

            } else {
                tempPoint.startTime = gameFile.hitObjects[i - 1].time - followPointFadeTime;
                tempPoint.startTime2 = gameFile.hitObjects[i - 1].time;
                tempPoint.startX = gameFile.hitObjects[i - 1].x;
                tempPoint.startY = gameFile.hitObjects[i - 1].y;
            }
            tempPoint.distance =
                std::sqrt(std::pow(std::abs(tempPoint.startX - tempPoint.endX), 2) +
                          std::pow(std::abs(tempPoint.startY - tempPoint.endY), 2));
            int numberOfPoints = tempPoint.distance / 32.0f;
            float offset = (tempPoint.distance - numberOfPoints * 32.0f) / 2.0f;
            for (int i = 0; i < numberOfPoints; i++) {
                float loc = offset + 16.0f + i * 32.0f;
                loc = loc / tempPoint.distance;
                Vector2 tempData = lerp({tempPoint.startX, tempPoint.startY},
                                        {tempPoint.endX, tempPoint.endY}, loc);
                tempPoint.points.push_back({tempData.x, tempData.y, loc});
            }
            followLines.push_back(tempPoint);
            // std::cout << "followLine from between times " << tempPoint.startTime << " - " <<
            // tempPoint.endTime << " and between cords " << tempPoint.startX << ", " <<
            // tempPoint.startY << " - " << tempPoint.endX << ", " << tempPoint.endY << "\n";
        }
    }

    std::reverse(followLines.begin(), followLines.end());
    std::reverse(gameFile.hitObjects.begin(), gameFile.hitObjects.end());

    reverse(times.begin(), times.end());

    defaultSampleSet = 0;
    if (gameFile.configGeneral["SampleSet"] == "Soft") {
        defaultSampleSet = 1;
    } else if (gameFile.configGeneral["SampleSet"] == "Drum") {
        defaultSampleSet = 2;
    }

    SoundFiles.data.clear();
    SoundFiles.loaded.clear();

    Global.loadingState = 3;

    Global.loadingState = 7;

    Global.Path = lastPath;
    Global.GameTextures = 1;
    std::cout << "mark text for loading" << std::endl;
    startMusic = true;
    stop = false;
}

void GameManager::unloadGame() {
    std::cout << "GameManager::unloadGame()" << std::endl;
    currentComboIndex = 0;
    Global.GameTextures = -1;

    for (auto &pair : SoundFiles.data) {
        UnloadSound(pair.second);
    }

    for (auto &pair : SoundFilesAll.data) {
        UnloadSound(pair.second);
    }

    UnloadMusicStream(backgroundMusic);
    free(musicData);

    Global.numberLines = -1;
    Global.parsedLines = -1;

    SoundFiles.data.clear();
    SoundFiles.loaded.clear();
    gameFile.hitObjects.clear();
    objects.clear();
    dead_objects.clear();
}

void GameManager::spawnHitObject(HitObjectData data) {
    // spawn a new hitobject, the cool way
    HitObject *temp;
    if (data.type == 1) {
        temp = new Circle(data);
    } else if (data.type == 2) {
        temp = new Slider(data);
    } else if (data.type == 3) {
        temp = new Spinner(data);
    } else {
        temp = new Circle(data);
    }

    objects.push_back(temp);
}

void GameManager::destroyHitObject(int index) {
    // declare a hitobject dead
    dead_objects.push_back(objects[index]);
    objects.erase(objects.begin() + index);
}

void GameManager::destroyDeadHitObject(int index) {
    // somehow "kill" the "dead" object
    /*if(dead_objects[index]->data.type == 2){
            Slider* tempslider = dynamic_cast<Slider*>(dead_objects[index]);
            if(tempslider->data.textureLoaded == true and tempslider->data.textureReady == false){
                    tempslider->renderedLocations.clear();
            tempslider->renderPoints.clear();
                    delete dead_objects[index];
                    dead_objects.erase(dead_objects.begin()+index);
            }
    }*/
    // else{
    delete dead_objects[index];
    dead_objects.erase(dead_objects.begin() + index);
    //}
}

void GameManager::render_points() {
    // garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
}

void GameManager::render_combo() {
    // garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
}

void GameManager::loadGameTextures() {
    Image tempImage = LoadImage("resources/sliderin.png");
    ImageColorReplace(&tempImage, {255, 255, 255, 159}, {255, 255, 255, 0});
    sliderin = LoadTextureFromImage(tempImage);
    UnloadImage(tempImage);

    Image tempImage2 = GenImageGradientRadial(sliderin.width, sliderin.height, 0.1,
                                              {255, 0, 0, 255}, {0, 0, 0, 0});
    sliderblank = LoadTexture("resources/SliderBlank.png");
    sliderout = LoadTextureFromImage(tempImage2);
    UnloadImage(tempImage2);
    // sliderout = LoadTexture("resources/sliderout.png");
    followPoint = LoadTexture("resources/followpoint.png");
    loadDefaultSkin(Global.selectedPath);  // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
    loadGameSkin(Global.selectedPath);     // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
    if (!Global.settings.useDefaultSkin) {
        loadBeatmapSkin(Global.selectedPath);  // LOADING THE BEATMAP SKIN USING A SEPERATE FUNCTION
    }
    SetTextureFilter(hit0, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(hit50, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(hit100, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(hit300, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(approachCircle, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(selectCircle, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(hitCircleOverlay, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(hitCircle, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(sliderb, TEXTURE_FILTER_BILINEAR);

    SetTextureFilter(sliderin, TEXTURE_FILTER_POINT);
    SetTextureFilter(sliderblank, TEXTURE_FILTER_POINT);
    SetTextureFilter(sliderout, TEXTURE_FILTER_POINT);
    SetTextureFilter(sliderscorepoint, TEXTURE_FILTER_BILINEAR);

    SetTextureFilter(reverseArrow, TEXTURE_FILTER_BILINEAR);
    for (int i = 0; i < 10; i++) {
        SetTextureFilter(numbers[i], TEXTURE_FILTER_BILINEAR);  // OPENGL1.1 DOESNT SUPPORT THIS
    }

    backgroundTextures.data.clear();
    backgroundTextures.pos.clear();
    backgroundTextures.loaded.clear();

    std::vector<std::string> files;
    files.clear();

    Global.Path = lastPath + '/';
    files = ls(".png");
    std::vector<std::string> files2 = ls(".jpg");
    std::vector<std::string> files3 = ls(".jpeg");
    files.insert(files.end(), files2.begin(), files2.end());
    files.insert(files.end(), files3.begin(), files3.end());

    for (int i = 0; i < (int)gameFile.events.size(); i++) {
        if (gameFile.events[i].eventType == 0) {
            std::cout << "Time: " << gameFile.events[i].startTime
                      << "ms - Filename: " << gameFile.events[i].filename << '.' << std::endl;
            if (gameFile.events[i].startTime < 1000) {
                gameFile.events[i].startTime -= 87000;
                std::cout << "Time changed to: " << gameFile.events[i].startTime << std::endl;
            }
        }
    }

    std::cout << "Found this many files: " << files.size() << std::endl;
    for (int i = 0; i < files.size(); i++) {
        std::cout << files[i] << std::endl;
    }

    for (int i = 0; i < files.size(); i++) {
        for (int j = 0; j < (int)gameFile.events.size(); j++) {
            if (gameFile.events[j].eventType == 0) {
                std::cout << "attempting to load a background\n";
                int t = gameFile.events[j].filename.size() - 1;
                while (gameFile.events[j].filename[t] == ' ' and t >= 0) {
                    gameFile.events[j].filename.pop_back();
                    t--;
                }
                t = 0;
                while (gameFile.events[j].filename[t] == ' ' and
                       gameFile.events[j].filename.size() > 0) {
                    gameFile.events[j].filename.erase(gameFile.events[j].filename.begin());
                }
                std::cout << "finding function returned: "
                          << files[i].rfind(gameFile.events[j].filename, 0)
                          << " for: " << gameFile.events[j].filename << " and " << files[i]
                          << std::endl;
                if (files[i].rfind(gameFile.events[j].filename, 0) == 0) {
                    std::cout << "WHAT DA HEEEEEEEEEELLLLLLLLLLLLL" << std::endl;
                    Image image = LoadImage((Global.Path + files[i]).c_str());
                    // ImageColorBrightness(&image, -128);
                    ImageColorTint(&image, Color{30, 30, 30, 255});
                    ImageBlurGaussian(&image, 2);

                    backgroundTextures.data[gameFile.events[j].filename] =
                        LoadTextureFromImage(image);
                    UnloadImage(image);

                    backgroundTextures.pos[gameFile.events[j].filename] = {
                        gameFile.events[j].xOffset, gameFile.events[j].yOffset};
                    if (backgroundTextures.data[gameFile.events[j].filename].width != 0) {
                        backgroundTextures.loaded[gameFile.events[j].filename].value = true;
                        std::cout << "Loaded: Background with filename: "
                                  << gameFile.events[j].filename << std::endl;
                        SetTextureFilter(backgroundTextures.data[gameFile.events[j].filename],
                                         TEXTURE_FILTER_BILINEAR);
                    }
                }
            }
        }
    }

    Global.GameTextures = 0;
}
void GameManager::unloadGameTextures() {
    std::cout << "UnloadingTextures" << std::endl;
    Global.GameTextures = 2;
    UnloadTexture(hitCircleOverlay);
    UnloadTexture(hitCircle);
    UnloadTexture(sliderscorepoint);
    UnloadTexture(approachCircle);
    UnloadTexture(hit300);
    UnloadTexture(hit100);
    UnloadTexture(hit50);
    UnloadTexture(hit0);
    UnloadTexture(sliderb);
    UnloadTexture(sliderin);
    UnloadTexture(sliderblank);
    UnloadTexture(sliderout);
    UnloadTexture(selectCircle);
    UnloadTexture(reverseArrow);
    UnloadTexture(spinnerBottom);
    UnloadTexture(spinnerTop);
    UnloadTexture(spinnerCircle);
    UnloadTexture(spinnerApproachCircle);
    UnloadTexture(spinnerMetre);
    UnloadTexture(followPoint);
    for (int i = 0; i < 10; i++) {
        UnloadTexture(numbers[i]);
    }
    std::string key;
    for (std::map<std::string, Texture2D>::iterator it = backgroundTextures.data.begin();
         it != backgroundTextures.data.end(); ++it) {
        key = it->first;
        std::cout << "Removed: " << it->first << "\n";
        UnloadTexture(backgroundTextures.data[key]);
    }

    backgroundTextures.data.clear();
    backgroundTextures.pos.clear();
    backgroundTextures.loaded.clear();
}

int orientation2(Vector2 &p1, Vector2 &p2, Vector2 &p3) {
    int val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
    return (val > 0) ? false : true;
}

float interpolate2(float *p, float *time, float t) {
    float L01 =
        p[0] * (time[1] - t) / (time[1] - time[0]) + p[1] * (t - time[0]) / (time[1] - time[0]);
    float L12 =
        p[1] * (time[2] - t) / (time[2] - time[1]) + p[2] * (t - time[1]) / (time[2] - time[1]);
    float L23 =
        p[2] * (time[3] - t) / (time[3] - time[2]) + p[3] * (t - time[2]) / (time[3] - time[2]);
    float L012 =
        L01 * (time[2] - t) / (time[2] - time[0]) + L12 * (t - time[0]) / (time[2] - time[0]);
    float L123 =
        L12 * (time[3] - t) / (time[3] - time[1]) + L23 * (t - time[1]) / (time[3] - time[1]);
    float C12 =
        L012 * (time[2] - t) / (time[2] - time[1]) + L123 * (t - time[1]) / (time[2] - time[1]);
    return C12;
}

std::vector<Vector2> interpolate2(std::vector<Vector2> &points, int index, int pointsPerSegment) {
    std::vector<Vector2> result;
    float x[4];
    float y[4];
    float time[4];
    for (int i = 0; i < 4; i++) {
        x[i] = points[index + i].x;
        y[i] = points[index + i].y;
        time[i] = i;
    }
    float tstart = 1;
    float tend = 2;
    float total = 0;
    for (int i = 1; i < 4; i++) {
        float dx = x[i] - x[i - 1];
        float dy = y[i] - y[i - 1];
        total += std::pow(dx * dx + dy * dy, .25);
        time[i] = total;
    }
    tstart = time[1];
    tend = time[2];
    int segments = pointsPerSegment - 1;
    result.push_back(points[index + 1]);
    for (int i = 1; i < segments; i++) {
        float xi = interpolate2(x, time, tstart + (i * (tend - tstart)) / segments);
        float yi = interpolate2(y, time, tstart + (i * (tend - tstart)) / segments);
        result.push_back(Vector2{xi, yi});
    }
    result.push_back(points[index + 2]);
    return result;
}

std::vector<Vector2> interpolate2(std::vector<Vector2> &coordinates, float length) {
    std::vector<Vector2> vertices;
    std::vector<int> pointsPerSegment;
    for (size_t i = 0; i < coordinates.size(); i++) {
        vertices.push_back(coordinates[i]);
        if (i > 0) pointsPerSegment.push_back(distance(vertices[i], vertices[i - 1]));
    }
    float lengthAll = 0;
    for (size_t i = 0; i < pointsPerSegment.size(); i++) lengthAll += pointsPerSegment[i];
    for (size_t i = 0; i < pointsPerSegment.size(); i++) pointsPerSegment[i] *= length / lengthAll;
    float dx = vertices[1].x - vertices[0].x;
    float dy = vertices[1].y - vertices[0].y;
    float x1 = vertices[0].x - dx;
    float y1 = vertices[0].y - dy;
    Vector2 start = {x1, y1};
    int n = vertices.size() - 1;
    dx = vertices[n].x - vertices[n - 1].x;
    dy = vertices[n].y - vertices[n - 1].y;
    float xn = vertices[n].x + dx;
    float yn = vertices[n].y + dy;
    Vector2 end = {xn, yn};
    vertices.insert(vertices.begin(), start);
    vertices.push_back(end);
    std::vector<Vector2> result;
    for (size_t i = 0; i < vertices.size() - 3; i++) {
        std::vector<Vector2> points = interpolate2(vertices, i, pointsPerSegment[i]);
        for (size_t i = (result.size() > 0) ? 1 : 0; i < points.size(); i++)
            result.push_back(points[i]);
    }
    return result;
}

std::vector<int> GameManager::sliderPreInit(HitObjectData data) {
    bool durationNull = false;
    double templength = data.length;
    if (data.length < 1) {
        data.length = 1;
        durationNull = true;
    }

    std::vector<Vector2> edgePoints;
    std::vector<Vector2> renderPoints;
    edgePoints.push_back(Vector2{(float)data.x, (float)data.y});

    float resolution = data.length;
    float currentResolution = 0;
    float lengthScale, totalLength = 0;

    Vector2 extraPosition;

    for (size_t i = 0; i < data.curvePoints.size(); i++)
        edgePoints.push_back(
            Vector2{(float)data.curvePoints[i].first, (float)data.curvePoints[i].second});

    if (edgePoints.size() == 1) {
        for (int k = 0; k < data.length; k++) {
            renderPoints.push_back(edgePoints[0]);
        }
    } else {
        if (data.curveType == 'L') {
            extraPosition = data.extraPos;
            edgePoints[edgePoints.size() - 1] = extraPosition;
            data.totalLength -= data.lengths[data.lengths.size() - 1];
            data.lengths[data.lengths.size() - 1] =
                std::sqrt(std::pow(std::abs(edgePoints[edgePoints.size() - 2].x -
                                            edgePoints[edgePoints.size() - 1].x),
                                   2) +
                          std::pow(std::abs(edgePoints[edgePoints.size() - 2].y -
                                            edgePoints[edgePoints.size() - 1].y),
                                   2));
            data.totalLength += data.lengths[data.lengths.size() - 1];
            lengthScale = data.totalLength / data.length;
            for (size_t i = 0; i < edgePoints.size() - 1; i++)
                for (float j = 0; j < data.lengths[i]; j += lengthScale)
                    renderPoints.push_back(
                        Vector2{edgePoints[i].x +
                                    (edgePoints[i + 1].x - edgePoints[i].x) * j / data.lengths[i],
                                edgePoints[i].y +
                                    (edgePoints[i + 1].y - edgePoints[i].y) * j / data.lengths[i]});
            renderPoints.push_back(edgePoints[edgePoints.size() - 1]);
            while (!false) {
                if (renderPoints.size() <= data.length) break;
                renderPoints.pop_back();
            }
        } else if (data.curveType == 'B') {
            Vector2 edges[edgePoints.size()];
            for (size_t i = 0; i < edgePoints.size(); i++) edges[i] = edgePoints[i];
            std::vector<Vector2> tempEdges;
            std::vector<Vector2> tempRender;
            std::vector<float> curveLengths;
            double totalCalculatedLength = 0;
            tempEdges.clear();
            tempRender.clear();
            int curveIndex = 0;
            double currentMax = 0;
            std::vector<Vector2> samples;
            std::vector<int> indices;
            std::vector<float> lengths;
            bool first = true;
            double tempResolution;
            for (size_t i = 0; i < edgePoints.size(); i++) {
                tempEdges.push_back(edgePoints[i]);
                if (i == edgePoints.size() - 1 || (edgePoints[i].x == edgePoints[i + 1].x &&
                                                   edgePoints[i].y == edgePoints[i + 1].y)) {
                    tempResolution =
                        data.lengths[curveIndex];  // clip(data.lengths[curveIndex], 0, 20000);
                    // std::cout << "tempResolution: " << tempResolution << std::endl;
                    tempResolution = std::min(data.lengths[curveIndex], 400.0f);
                    if (tempResolution > 0 and tempEdges.size() > 1) {
                        if (first) {
                            samples.push_back(get2BezierPoint(tempEdges, tempEdges.size(), 0));
                            lengths.push_back(0);
                        }
                        int lastk = 0;
                        int k = 1;
                        if (!first) k = 0;
                        for (; k < tempResolution; k++) {
                            samples.push_back(get2BezierPoint(tempEdges, tempEdges.size(),
                                                              ((double)k) / tempResolution));
                            lengths.push_back(
                                distance(samples[samples.size() - 1], samples[samples.size() - 2]) +
                                lengths[lengths.size() - 1]);
                            lastk = k;
                        }

                        samples.push_back(get2BezierPoint(tempEdges, tempEdges.size(), 1));
                        lengths.push_back(
                            distance(samples[samples.size() - 1], samples[samples.size() - 2]) +
                            lengths[lengths.size() - 1]);
                        if (first) first = false;
                    }
                    curveIndex++;
                    tempEdges.clear();
                }
            }

            totalCalculatedLength = lengths[lengths.size() - 1];
            tempResolution = data.length;
            if (totalCalculatedLength < data.length) {
                float angle = atan2(samples[samples.size() - 1].y - samples[samples.size() - 2].y,
                                    samples[samples.size() - 1].x - samples[samples.size() - 2].x) *
                              180 / 3.14159265;
                float hipotenus = data.length - totalCalculatedLength;
                float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
                float ydiff = sqrt(std::abs(hipotenus * hipotenus - xdiff * xdiff));
                int ything = 1;
                if (angle < 0.0f) {
                    ything = -1;
                } else if (angle == 0.0f) {
                    ything = 0;
                }

                Vector2 extraPosition = {samples[samples.size() - 1].x + xdiff,
                                         samples[samples.size() - 1].y - ydiff * (float)ything};
                samples.push_back(extraPosition);
                lengths.push_back(
                    distance(samples[samples.size() - 1], samples[samples.size() - 2]) +
                    lengths[lengths.size() - 1]);
            }
            renderPoints.clear();
            int SampleIndex = 1;
            for (int index = 0; index <= data.length; index++) {
                while (index > lengths[SampleIndex]) {
                    if (SampleIndex == lengths.size() - 1)
                        break;
                    else {
                        SampleIndex++;
                    }
                }
                double lerpPos = (index - lengths[SampleIndex - 1]) /
                                 (lengths[SampleIndex] - lengths[SampleIndex - 1]);
                renderPoints.push_back(
                    lerp(samples[SampleIndex], samples[SampleIndex - 1], lerpPos));
            }
        } else if (data.curveType == 'P') {
            std::pair<Vector2, float> circleData =
                get2PerfectCircle(edgePoints[0], edgePoints[1], edgePoints[2]);
            float inf = std::numeric_limits<float>::infinity();
            if (circleData.first.x == -inf or circleData.first.x == inf or
                circleData.first.y == -inf or circleData.first.y == inf) {
                extraPosition = data.extraPos;
                edgePoints[edgePoints.size() - 1] = extraPosition;
                data.totalLength -= data.lengths[data.lengths.size() - 1];
                data.lengths[data.lengths.size() - 1] =
                    std::sqrt(std::pow(std::abs(edgePoints[edgePoints.size() - 2].x -
                                                edgePoints[edgePoints.size() - 1].x),
                                       2) +
                              std::pow(std::abs(edgePoints[edgePoints.size() - 2].y -
                                                edgePoints[edgePoints.size() - 1].y),
                                       2));
                data.totalLength += data.lengths[data.lengths.size() - 1];

                lengthScale = data.totalLength / data.length;

                for (size_t i = 0; i < edgePoints.size() - 1; i++)
                    for (float j = 0; j < data.lengths[i]; j += lengthScale)
                        renderPoints.push_back(
                            Vector2{edgePoints[i].x + (edgePoints[i + 1].x - edgePoints[i].x) * j /
                                                          data.lengths[i],
                                    edgePoints[i].y + (edgePoints[i + 1].y - edgePoints[i].y) * j /
                                                          data.lengths[i]});
                renderPoints.push_back(edgePoints[edgePoints.size() - 1]);
                while (!false) {
                    if (renderPoints.size() <= data.length) break;
                    renderPoints.pop_back();
                }
            } else {
                Vector2 center = circleData.first;
                int radius = circleData.second;
                float degree1 =
                    atan2(edgePoints[0].y - center.y, edgePoints[0].x - center.x) * RAD2DEG;
                float degree2 =
                    atan2(edgePoints[1].y - center.y, edgePoints[1].x - center.x) * RAD2DEG;
                float degree3 =
                    atan2(edgePoints[2].y - center.y, edgePoints[2].x - center.x) * RAD2DEG;
                degree1 = degree1 < 0 ? degree1 + 360 : degree1;
                degree2 = degree2 < 0 ? degree2 + 360 : degree2;
                degree3 = degree3 < 0 ? degree3 + 360 : degree3;
                bool clockwise = !orientation2(edgePoints[0], edgePoints[1], edgePoints[2]);
                float angle = (((data.length * 360) / radius) / 3.14159265) / 2;
                int a = 0;
                if (clockwise) {
                    degree1 = degree1 < degree3 ? degree1 + 360 : degree1;
                    degree2 = degree2 < degree3 ? degree2 + 360 : degree2;
                    for (float i = degree1; i > degree1 - angle; i -= angle / data.length) {
                        if (a > data.length) {
                            renderPoints.pop_back();
                            break;
                        }
                        Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius,
                                                    center.y + sin(i / RAD2DEG) * radius};
                        renderPoints.push_back(tempPoint);
                        a++;
                    }
                } else {
                    degree2 = degree2 < degree1 ? degree2 + 360 : degree2;
                    degree3 = degree3 < degree1 ? degree3 + 360 : degree3;
                    for (float i = degree1; i < degree1 + angle; i += angle / data.length) {
                        if (a > data.length) {
                            renderPoints.pop_back();
                            break;
                        }
                        Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius,
                                                    center.y + sin(i / RAD2DEG) * radius};
                        renderPoints.push_back(tempPoint);
                        a++;
                    }
                    // std::reverse(renderPoints.begin(), renderPoints.end());
                }
                while (renderPoints.size() > data.length) {
                    renderPoints.pop_back();
                }
                // std::cout << "Pdata: " << data.length << " size: " << renderPoints.size() <<
                // std::endl;
            }

        } else if (data.curveType == 'C') {
            renderPoints = interpolate2(edgePoints, data.length);
            while (!false) {
                if (renderPoints.size() <= data.length) break;
                renderPoints.pop_back();
            }
        } else {
            std::__throw_invalid_argument("Invalid Slider type!");
        }
    }
    for (size_t i = 0; i < renderPoints.size(); i++) {
        if (renderPoints[i].x < -150) {
            renderPoints[i].x = -150;
        }
        if (renderPoints[i].y < -150) {
            renderPoints[i].y = -150;
        }
        if (renderPoints[i].x > 790) {
            renderPoints[i].x = 790;
        }
        if (renderPoints[i].y > 630) {
            renderPoints[i].y = 630;
        }
    }

    if (data.slides % 2 == 0) {
        data.ex = data.x;
        data.ey = data.y;
    } else {
        data.ex = renderPoints[renderPoints.size() - 1].x;
        data.ey = renderPoints[renderPoints.size() - 1].y;
    }

    float tempTimeLength = (data.length / 100) * (data.timing.beatLength) /
                           (sliderSpeed * data.timing.sliderSpeedOverride) * data.slides;
    float endTime = data.time + tempTimeLength - (36 - (18 * (tempTimeLength <= 72.0f)));
    if (durationNull) {
        endTime = data.time;
    }

    std::vector<int> out;

    edgePoints.clear();
    renderPoints.clear();

    out.push_back(data.ex);
    out.push_back(data.ey);
    out.push_back(endTime);

    return out;
}

void GameManager::loadGameSounds() {
    long long int loadedBytes = 0;
    SoundFilesAll.data.clear();
    SoundFilesAll.loaded.clear();

    std::string last = Global.Path;
    std::string dontTouch = gameFile.configGeneral["AudioFilename"];
    Global.Path = GamePathWithSlash;

    std::vector<std::string> ComboBreak = ls(".wav");
    if (Global.settings.useDefaultSounds) ComboBreak.clear();
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from game" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind(dontTouch, 0) != 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            if (name.rfind("drum", 0) == 0 or name.rfind("soft", 0) == 0 or
                name.rfind("normal", 0) == 0) {
                if (name[name.size() - 1] == 'l' or name[name.size() - 1] == 'e' or
                    name[name.size() - 1] == 'h' or name[name.size() - 1] == 'p') {
                    name += '1';
                }
            }
            SoundFilesAll.data[name] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
            SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
            if (SoundFilesAll.loaded[name].value) {
                std::filesystem::path p{Global.Path + ComboBreak[i]};
                loadedBytes += std::filesystem::file_size(p);
                std::cout << "loaded " << name << " from game" << std::endl;
            }
        }
    }
    ComboBreak = ls(".ogg");
    if (Global.settings.useDefaultSounds) ComboBreak.clear();
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from game" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind(dontTouch, 0) != 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            if (name.rfind("drum", 0) == 0 or name.rfind("soft", 0) == 0 or
                name.rfind("normal", 0) == 0) {
                if (name[name.size() - 1] == 'l' or name[name.size() - 1] == 'e' or
                    name[name.size() - 1] == 'h' or name[name.size() - 1] == 'p') {
                    name += '1';
                }
            }
            SoundFilesAll.data[name] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
            SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
            if (SoundFilesAll.loaded[name].value) {
                std::filesystem::path p{Global.Path + ComboBreak[i]};
                loadedBytes += std::filesystem::file_size(p);
                std::cout << "loaded " << name << " from game" << std::endl;
            }
        }
    }
    ComboBreak = ls(".mp3");
    if (Global.settings.useDefaultSounds) ComboBreak.clear();
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from game" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind(dontTouch, 0) != 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            if (name.rfind("drum", 0) == 0 or name.rfind("soft", 0) == 0 or
                name.rfind("normal", 0) == 0) {
                if (name[name.size() - 1] == 'l' or name[name.size() - 1] == 'e' or
                    name[name.size() - 1] == 'h' or name[name.size() - 1] == 'p') {
                    name += '1';
                }
            }
            SoundFilesAll.data[name] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
            SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
            if (SoundFilesAll.loaded[name].value) {
                std::filesystem::path p{Global.Path + ComboBreak[i]};
                loadedBytes += std::filesystem::file_size(p);
                std::cout << "loaded " << name << " from game" << std::endl;
            }
        }
    }

    Global.Path = "resources/skin/";
    ComboBreak = ls(".wav");
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from skin" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or
                   ComboBreak[i].rfind("normal", 0) == 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            while (std::isdigit(name[name.size() - 1])) name.pop_back();
            if (SoundFilesAll.loaded.count(name) == 0 or
                SoundFilesAll.loaded[name].value == false) {
                SoundFilesAll.data[name] = LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
                if (SoundFilesAll.loaded[name].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << name << " from skin" << std::endl;
                }
            }
        }
    }
    ComboBreak = ls(".ogg");
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from skin" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or
                   ComboBreak[i].rfind("normal", 0) == 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            while (std::isdigit(name[name.size() - 1])) name.pop_back();
            if (SoundFilesAll.loaded.count(name) == 0 or
                SoundFilesAll.loaded[name].value == false) {
                SoundFilesAll.data[name] = LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
                if (SoundFilesAll.loaded[name].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << name << " from skin" << std::endl;
                }
            }
        }
    }
    ComboBreak = ls(".mp3");
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from skin" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or
                   ComboBreak[i].rfind("normal", 0) == 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            while (std::isdigit(name[name.size() - 1])) name.pop_back();
            if (SoundFilesAll.loaded.count(name) == 0 or
                SoundFilesAll.loaded[name].value == false) {
                SoundFilesAll.data[name] = LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
                if (SoundFilesAll.loaded[name].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << name << " from skin" << std::endl;
                }
            }
        }
    }

    Global.Path = "resources/default_skin/";
    ComboBreak = ls(".wav");
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or
                   ComboBreak[i].rfind("normal", 0) == 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            while (std::isdigit(name[name.size() - 1])) name.pop_back();
            if (SoundFilesAll.loaded.count(name) == 0 or
                SoundFilesAll.loaded[name].value == false) {
                SoundFilesAll.data[name] =
                    LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
                if (SoundFilesAll.loaded[name].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << name << " from default skin" << std::endl;
                }
            }
        }
    }
    ComboBreak = ls(".ogg");
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or
                   ComboBreak[i].rfind("normal", 0) == 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            while (std::isdigit(name[name.size() - 1])) name.pop_back();
            if (SoundFilesAll.loaded.count(name) == 0 or
                SoundFilesAll.loaded[name].value == false) {
                SoundFilesAll.data[name] =
                    LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
                if (SoundFilesAll.loaded[name].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << name << " from default skin" << std::endl;
                }
            }
        }
    }
    ComboBreak = ls(".mp3");
    for (int i = 0; i < ComboBreak.size(); i++) {
        if (ComboBreak[i].rfind("combobreak", 0) == 0) {
            if (SoundFilesAll.loaded.count("combobreak") == 0 or
                SoundFilesAll.loaded["combobreak"].value == false) {
                SoundFilesAll.data["combobreak"] =
                    LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded["combobreak"].value =
                    IsSoundReady(SoundFilesAll.data["combobreak"]);
                if (SoundFilesAll.loaded["combobreak"].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
                }
            }
        } else if (ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or
                   ComboBreak[i].rfind("normal", 0) == 0) {
            std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
            while (std::isdigit(name[name.size() - 1])) name.pop_back();
            if (SoundFilesAll.loaded.count(name) == 0 or
                SoundFilesAll.loaded[name].value == false) {
                SoundFilesAll.data[name] =
                    LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
                SoundFilesAll.loaded[name].value = IsSoundReady(SoundFilesAll.data[name]);
                if (SoundFilesAll.loaded[name].value) {
                    std::filesystem::path p{Global.Path + ComboBreak[i]};
                    loadedBytes += std::filesystem::file_size(p);
                    std::cout << "loaded " << name << " from default skin" << std::endl;
                }
            }
        }
    }

    // gameFile.configGeneral["AudioFilename"]

    /*SetSoundVolume(SoundFilesAll.data["combobreak"], 1.0f);
    PlaySound(SoundFilesAll.data["combobreak"]);*/
    std::cout << "loaded " << loadedBytes / 1024 << "KB of sound data" << std::endl;
    Global.Path = last;
}
