// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <array>
#include "Math/Color.h"

/**
 * 
 */
class UNREALTIAOQI_API ColorConst
{
public:
	//static constexpr auto BLACK = FLinearColor(0.0, 0.0, 0.0, 1.0);
	//static constexpr auto WHITE = FLinearColor(1.0, 1.0, 1.0, 1.0);
	//static constexpr auto RED = FLinearColor(1.0, 0.0, 0.0, 1.0);
	//static constexpr auto GREEN = FLinearColor(0.0, 1.0, 0.0, 1.0);
	//static constexpr auto BLUE = FLinearColor(0.0, 0.0, 1.0, 1.0);
	static constexpr FLinearColor COLOR_0 = FLinearColor(0.08, 0.08, 0.08, 1);
	static constexpr FLinearColor COLOR_1 = FLinearColor(0.95, 0.95, 0.95, 1);
	static constexpr FLinearColor COLOR_2 = FLinearColor(0.83, 0.23, 0.23, 1);  
	static constexpr FLinearColor COLOR_3 = FLinearColor(0.17, 0.59, 0.86, 1);  
	static constexpr FLinearColor COLOR_4 = FLinearColor(0.48, 0.87, 0.23, 1);  
	static constexpr FLinearColor COLOR_5 = FLinearColor(0.93, 0.85, 0.23, 1);  
	static constexpr std::array<FLinearColor, 6> COLOR_ARRAY = { COLOR_0 ,COLOR_1 ,COLOR_2 ,COLOR_3 ,COLOR_4 ,COLOR_5 };
	static FLinearColor randomColor() {
		return COLOR_ARRAY[FMath::RandRange(0, 5)];
	}
	ColorConst() {};
	~ColorConst() {};
};
