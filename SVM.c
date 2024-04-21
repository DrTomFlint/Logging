
/** Space Vector Pulse Width Modulation.
 *
 * This function reads the normalized output voltages in alpha-beta
 * frame and calculates the on-times for the 3 PWM duty cycle registers
 * using one of several methods.
 *
 * Inputs:
 * + SvmAlpha   normalized reference voltage in alpha
 * + SvmBeta    normalized reference voltage in beta
 * + SvmPeriod  integer counts in a pwm period
 * + SvmMethod  integer 1=symmetric, 2=bus clamp 60 degree odd, 3=bus clamp 60 degree even.
 *
 * Outputs:
 * + SvmOnA	  on time for pwm duty register A
 * + SvmOnB	  on time for pwm duty register B
 * + SvmOnC	  on time for pwm duty register C
 * + SvmClip	  flag to indicate clipping
 * + SvmK		  clipping coefficient on magnitude of Vref
 *
 */
#pragma CODE_SECTION(UpdateSpaceVector, "ramfuncs");
void UpdateSpaceVector(void){

	// Compute some scaled values
	SvmBetaSqrt3 = SvmBeta * RECIP_SQRT3;
	SvmAlphaAbs = fabs(SvmAlpha);
	SvmSector = 0;

	// Determine which sector based on alpha-beta
	if(SvmBeta>=0){
		// Must be sector 1,2,3
		if(SvmAlphaAbs < fabs(SvmBetaSqrt3)){
			SvmSector = 2;
		}else{
			if(SvmAlpha>=0){
				SvmSector = 1;
			}else{
				SvmSector = 3;
			}
		}
	}else{
		// Must be sector 4,5,6
		if(SvmAlphaAbs < fabs(SvmBetaSqrt3)){
			SvmSector = 5;
		}else{
			if(SvmAlpha>=0){
				SvmSector = 6;
			}else{
				SvmSector = 4;
			}
		}

	}

	// Calculate on-times for active vectors X and Y
	if((SvmSector==2)||(SvmSector==5)){
		// group 1
		SvmTx = SvmAlpha + fabs(SvmBetaSqrt3);
		SvmTy = -SvmAlpha + fabs(SvmBetaSqrt3);
	}else{
		SvmTx = fabs(SvmAlpha)-fabs(SvmBetaSqrt3);
		SvmTy = 2.0*fabs(SvmBetaSqrt3);
	}

	// Calculate zero time, rescale Tx and Ty if in overmodulation
	// Set or reset the flag SvmClip
	SvmT0 = 1.0 - SvmTx - SvmTy;
	if(SvmT0<0){
		SvmT0 = 0;
		SvmClip = 1;
		SvmK = 1.0 / (SvmTx+SvmTy);
		SvmTx = SvmK * SvmTx;
		SvmTy = SvmK * SvmTy;
	}else{
		SvmClip = 0;
		SvmK = 1.0;
	}

	// Assign on-times based on Tx, Ty, and T0 times combined with sector
	if(SvmMethod == 1){
		// Standard symmetric SVM
		SvmT02 = SvmT0 * 0.5;
		switch(SvmSector){
		case 1:
			SvmOnA = SvmPeriod*(SvmTx + SvmTy + SvmT02);
			SvmOnB = SvmPeriod*(SvmTy + SvmT02);
			SvmOnC = SvmPeriod*(SvmT02);
			break;
		case 2:
			SvmOnA = SvmPeriod*(SvmTx + SvmT02);
			SvmOnB = SvmPeriod*(SvmTx + SvmTy + SvmT02);
			SvmOnC = SvmPeriod*(SvmT02);
			break;
		case 3:
			SvmOnA = SvmPeriod*(SvmT02);
			SvmOnB = SvmPeriod*(SvmTx + SvmTy + SvmT02);
			SvmOnC = SvmPeriod*(SvmTx + SvmT02);
			break;
		case 4:
			SvmOnA = SvmPeriod*(SvmT02);
			SvmOnB = SvmPeriod*(SvmTx + SvmT02);
			SvmOnC = SvmPeriod*(SvmTx + SvmTy + SvmT02);
			break;
		case 5:
			SvmOnA = SvmPeriod*(SvmTx + SvmT02);
			SvmOnB = SvmPeriod*(SvmT02);
			SvmOnC = SvmPeriod*(SvmTx + SvmTy + SvmT02);
			break;
		case 6:
			SvmOnA = SvmPeriod*(SvmTx + SvmTy + SvmT02);
			SvmOnB = SvmPeriod*(SvmT02);
			SvmOnC = SvmPeriod*(SvmTy + SvmT02);
			break;
		default:			// just for safety, should never execute
			SvmOnA = 0;
			SvmOnB = 0;
			SvmOnC = 0;
			break;
		}
	}

#if(0)	// May want to exclude from executable to gain codespace
	if(SvmMethod == 2){
		// Bus Clamped 60 degree odd SVM
		switch(SvmSector){
		case 1:
			SvmOnA = SvmPeriod;
			SvmOnB = SvmPeriod*(SvmTy + SvmT0);
			SvmOnC = SvmPeriod*(SvmT0);
			break;
		case 2:
			SvmOnA = SvmPeriod*(SvmTx);
			SvmOnB = SvmPeriod*(SvmTx + SvmTy);
			SvmOnC = 0;
			break;
		case 3:
			SvmOnA = SvmPeriod*(SvmT0);
			SvmOnB = SvmPeriod;
			SvmOnC = SvmPeriod*(SvmTx + SvmT0);
			break;
		case 4:
			SvmOnA = 0;
			SvmOnB = SvmPeriod*(SvmTx);
			SvmOnC = SvmPeriod*(SvmTx + SvmTy);
			break;
		case 5:
			SvmOnA = SvmPeriod*(SvmTx + SvmT0);
			SvmOnB = SvmPeriod*(SvmT0);
			SvmOnC = SvmPeriod;
			break;
		case 6:
			SvmOnA = SvmPeriod*(SvmTx + SvmTy);
			SvmOnB = 0;
			SvmOnC = SvmPeriod*(SvmTy);
			break;
		default:			// just for safety, should never execute
			SvmOnA = 0;
			SvmOnB = 0;
			SvmOnC = 0;
			break;
		}
	}
#endif

#if(0)	// May want to exclude from executable to gain codespace
	if(SvmMethod == 3){
		// Bus Clamped 60 degree even SVM
		switch(SvmSector){
		case 1:
			SvmOnA = SvmPeriod*(SvmTx + SvmTy);
			SvmOnB = SvmPeriod*(SvmTy);
			SvmOnC = 0;
			break;
		case 2:
			SvmOnA = SvmPeriod*(SvmTx + SvmT0);
			SvmOnB = SvmPeriod;
			SvmOnC = SvmPeriod*(SvmT0);
			break;
		case 3:
			SvmOnA = 0;
			SvmOnB = SvmPeriod*(SvmTx + SvmTy);
			SvmOnC = SvmPeriod*(SvmTx);
			break;
		case 4:
			SvmOnA = SvmPeriod*(SvmT0);
			SvmOnB = SvmPeriod*(SvmTx + SvmT0);
			SvmOnC = SvmPeriod;
			break;
		case 5:
			SvmOnA = SvmPeriod*(SvmTx);
			SvmOnB = 0;
			SvmOnC = SvmPeriod*(SvmTx + SvmTy);
			break;
		case 6:
			SvmOnA = SvmPeriod;
			SvmOnB = SvmPeriod*(SvmT0);
			SvmOnC = SvmPeriod*(SvmTy + SvmT0);
			break;
		default:			// just for safety, should never execute
			SvmOnA = 0;
			SvmOnB = 0;
			SvmOnC = 0;
			break;
		}
	}
#endif

#if(0)
	// deadtime compensation
	if(Ia>0){
		SvmOnA = SvmOnA + SvmDtc;
		if(SvmOnA>SvmPeriod)SvmOnA=SvmPeriod;
	}else{
		SvmOnA = SvmOnA - SvmDtc;
		if(SvmOnA<0)SvmOnA=0;
	}

	if(Ib>0){
		SvmOnB = SvmOnB + SvmDtc;
		if(SvmOnB>SvmPeriod)SvmOnB=SvmPeriod;
	}else{
		SvmOnB = SvmOnB - SvmDtc;
		if(SvmOnB<0)SvmOnB=0;
	}

	if(Ic>0){
		SvmOnC = SvmOnC + SvmDtc;
		if(SvmOnC>SvmPeriod)SvmOnC=SvmPeriod;
	}else{
		SvmOnC = SvmOnC - SvmDtc;
		if(SvmOnC<0)SvmOnC=0;
	}
#endif
}
