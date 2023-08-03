#ifdef STEPPER


/*    

            if (stepbuf.size())
            {
                stepcmd xx = stepbuf.back();
                stepbuf.pop_back();
                digitalWrite(enables[xx.id], LOW);

                mstep[xx.id].setSpeedInStepsPerSecond(xx.speed);
                mstep[xx.id].setAccelerationInStepsPerSecondPerSecond(xx.accel);
                mstep[xx.id].relativeMoveInSteps(xx.pos);

                vTaskDelay(10);
                //            digitalWrite(enables[xx.id], HIGH);
                sendEvent("Moved");

                // MOVE
            }
        vTaskDelay(10);
    }
}

*/
#endif
