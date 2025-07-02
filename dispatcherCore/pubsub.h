// core data structure to keep track of publishers and subscribers

#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#include <memory>
#include "config.h"

typedef struct publisherDBentry_ {
    char pubName[64];
    uint32_t publisherId;
    uint32_t publishedMsgIds[MAX_PUBLISHED_MSG];

    publisherDBentry_() {
        pubName[0] = '\0';
        publisherId = 0;
        memset(publishedMsgIds, 0, sizeof(publishedMsgIds));
    }
} publisherDBentry_t;

typedef struct subscriberDBentry_ {
    char subName[64];
    uint32_t subscriberId;
    uint32_t subscriberMsgIds[MAX_SUBSCRIBED_MSG];

    subscriberDBentry_() {
        subName[0] = '\0';
        subscriberId = 0;
        memset(subscriberMsgIds, 0, sizeof(subscriberMsgIds));
    }
} subscriberDBentry_t;

typedef struct pubSubDBentry_ {
    uint32_t publishMsgCode;
    std::vector<std::shared_ptr<subscriberDBentry_t>> subscribers;

    pubSubDBentry_() {
        publishMsgCode = 0;
        subscribers.clear();
    }
} pubSubDBentry_t;