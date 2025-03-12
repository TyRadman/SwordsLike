#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUtility, Log, All);

#define PRINT_LOG(Message, ...) UE_LOG(LogUtility, Log, TEXT(Message), ##__VA_ARGS__)
#define PRINT_WARNING(Message, ...) UE_LOG(LogUtility, Warning, TEXT(Message), ##__VA_ARGS__)
#define PRINT_ERROR(Message, ...) UE_LOG(LogUtility, Error, TEXT(Message), ##__VA_ARGS__)
