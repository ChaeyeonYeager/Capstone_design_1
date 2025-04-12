#ifndef FEED_LEVEL_CHECK_H
#define FEED_LEVEL_CHECK_H

void setupFeedingSystem();
void handleFeedingLogic();
float measureDistance(int trig, int echo);
float getAverageDistance();
void checkFoodLevel();
bool isFoodInputDone();

#endif
