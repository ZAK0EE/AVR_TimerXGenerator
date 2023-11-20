#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <ctype.h>



//**************************Simple helpful functions**************************

//Checks if the given number is integer.
int check_int(float var) {return (int)var == var;}
//Multiplies the time by 1000 until it's over one.(used for some calculations.)
int over_one(float var){return (var > 1)? var: over_one(var * 1000);}

//**************************Functions prototypes**************************

//Return (string) time using 'second' unit.
char* print_unit(float b);
//Return (string) the decimal number into binary.
char* print_binary( uint8_t a);
//Return (string) the decimal number into hexa.
char* print_hexa(uint8_t num);
//**************************Main program**************************
int main()
{


    while(1)
    {
        //Variables.
        uint32_t freq = 0; //Device clock frequency.
        uint8_t reg_c = 0; //Control register.
        float min_t = 0; //Minimum value for any timer.
        float TimerX_Max[3] = {0}; //Maximum value for all timers.
        float time_delay = 0;
        char ch = 0;
        system("CLS");
        //**************************Big title**************************
        //https://www.messletters.com/en/big-text/  | This website used for generating this text below.
        printf("\t\t\t\t   ____           ____     ___    _   _      _      ____  \n");
        printf("\t\t\t\t  / ___|         / ___|   / _ \\  | | | |    / \\    |  _ \\ \n");
        printf("\t\t\t\t | |             \\___ \\  | | | | | | | |   / _ \\   | | | |\n");
        printf("\t\t\t\t | |___           ___) | | |_| | | |_| |  / ___ \\  | |_| |\n");
        printf("\t\t\t\t  \\____|  _____  |____/   \\__\\_\\  \\___/  /_/   \\_\\ |____/ \n");
        printf("\t\t\t\t         |_____|                                          \n\n");

        printf("\t\t\t\t     This program was brought to you by C_Squad team\n");
        printf("\t\t\t\t\t  part of X_Squad robotic team in Tanta\n");
        printf("\t\t\t\t      ATmega16 TimerX function generator by zezcode\n\n\n");

        //**************************Program start**************************
        printf("Pleas enter the clock frequency of your device: ");
        //Safety measures, prevent the user from entering weird characters.
        while(!scanf("%d", &freq))
        {
            printf("Please enter a valid number!\n");
            printf("Pleas enter the clock frequency of your device: ");
            fflush(stdin);
        }
        //Minimum and Max time calculations.
        min_t = 1.0 / freq;
        TimerX_Max[1] = 0xFFFF * (1024.0 / freq);
        TimerX_Max[0] = TimerX_Max[2] = 0xFF * (1024.0 /freq);

        //Safety measures, prevent the user from entering weird characters.
        //Time_Delay input.
         do
        {
            //Time input by user.
            printf("Please enter the required delay time as float( %s -> %0.0fs ):  ", print_unit(min_t), TimerX_Max[1]);
            fflush(stdin);
        }
        while((!scanf("%f", &time_delay)) || time_delay < min_t || time_delay > TimerX_Max[1] );

        //This gonna be the ticks number (no pre-scale for now).
        int value_num = time_delay * freq;
        //Choice -> Number of the timer, choice_m -> number of the timer mode.
        int choice = 0, choice_m = 0;

        //This array to show which timers are available.
        uint8_t t_c[3] = {0};

        //Checking which timer is possible for the operation.
        do
        {
            fflush(stdin);
            printf("Please enter the number of the timer you wish to use ( ");
            if (time_delay < TimerX_Max[0])
            {
                //If time_delay is not of 2 powers, recommend timer1.
                //The over_one to raise the time over one to be able to get the log.
                if (check_int(log(over_one(time_delay))/ log(2)) || value_num < 0xFF )
                    printf("Timer0, Timer1, Timer2 ): ");
                else
                    printf("Timer0, Timer1 (Recommended for accuracy), Timer2 ): ");//Means only Timer1 will give integer number of ticks.
                //All timers are available.
                t_c[0] = t_c[1] = t_c[2] = 1;
            }
            else
            {
                printf("Timer1 ) : ");
                //Timer1 is the only available timer.
                t_c[1] = 1;
            }

        }
        while(!scanf("%d", &choice) || choice < 0 || choice > 2 || !t_c[choice]);

        //Only Timer0 and Timer1 can use external clock.
        if (choice == 1 || choice == 0)
        {
            do
            {
                fflush(stdin);
                printf("Are you using external clock? (y, n): ");
            }
            while(!scanf("%c", &ch) || (tolower(ch) != 'y'&& tolower(ch) != 'n') );
        }

        if (ch == 'y')
        {
              do
            {
                fflush(stdin);
                printf("y :falling edge, n : rising edge.(y, n): ");
            }
            while(!scanf("%c", &ch) || (tolower(ch) != 'y'&& tolower(ch) != 'n') );
            //Filling control register with required bits for external clock.
            if (ch == 'y')
            {
                reg_c |= (1 << 1) | (1 << 2);
            }
            else
                reg_c |= (1 << 0) |(1 << 1) | (1 << 2);
        }


        //Mode decision.
        do
        {
            printf("Please choose which method you wish to use(0-Overflow, 1-CTC): ");
        }
        while(!scanf("%d", &choice_m) || choice_m < 0 || choice_m > 1);
        //The CTC bit for all timers is the same
        if (choice_m == 1)
        {
            reg_c |= (1 << 3);
        }

        //Checking if Pre-scale required.
        //0xFF is the max for timer 0 and 2, if the value_Num is lower than max, no need for pre-scale
        if (value_num > ((choice == 0 || choice == 2) ? 0xFF : 0xFFFF))
        {
            if (choice == 1 || choice == 0)
            {
                int scal[4] = {8, 64, 256, 1024};
                for (int i = 0, scval = 2; i < 4; i++, scval++)
                {
                    //Trying to divide to all scales in attemps to get an integer value, if not, just use 1024 scale.
                    if (value_num % scal[i] == 0 && value_num / scal[i] <= ((choice == 0)? 0xFF : 0xFFFF)) //If timer0 then max is 0xFF, and for timer1 max is 0xFFFF.
                    {

                        reg_c |= scval;//Loads the CTC in the control register "TCCRx".
                        value_num /= scal[i]; //Apply the pre-scale
                        break;
                    }
                    if (i == 3) //if no integer output, just use 1024 pre-scale
                    {
                        reg_c |= 5;
                        value_num /= scal[3];
                    }
                }
            }
            else if (choice == 2)
            {
                int scal[6] = {8, 32, 64, 128, 256, 1024};
                for (int i = 0, scval = 2; i < 6; i++, scval++)
                {
                    //Trying to divide to all scales in attemps to get an integer value, if not, just use 1024 scale.
                    if (value_num % scal[i] == 0 && value_num / scal[i] <= 0xFF)
                    {
                        reg_c |= scval;//Loads the CTC in the control register "TCCRx".
                        value_num /= scal[i];//Apply the pre-scale
                        break;
                    }
                    if (i == 5)//if no integer output, just use 1024 pre-scale
                    {
                        reg_c |= 7;
                        value_num /= scal[5];
                    }
                }
            }
        }

        printf("Proceed to the function...\n");
        system("PAUSE");
        system("CLS"); //Clear the screen.

        //*******************************Writing the function*******************************
        //Choice refers to the timer.
        if (choice == 0 || choice == 2)
        {
            printf("void delay_%s()\n{\n", (choice_m == 0)? "OV" : "CTC");
            printf("\t// using Timer%d to generate time delay: %s;\n", choice, print_unit(time_delay));
            printf("\tTCNT%d = %s;\n", choice, (choice_m == 0)? print_hexa(0xFF - value_num + 1) : "0x00" );
            if(choice_m == 1) printf("\tOCR%d = %s;\n", choice, print_hexa(value_num - 1) ); //Only in CTC mode.
            printf("\tTCCR%d = 0b%s;\n", choice, print_binary(reg_c));
            printf("\twhile((TIFR & (1<<%s%d)) == 0);\n", (choice_m == 0)? "TOV" : "OCF" ,choice);
            printf("\tTIFR |= (1<<%s%d);\n",(choice_m == 0)? "TOV" : "OCF" , choice);
            printf("\tTCCR%d = 0;\n", choice);
            printf("}\n");

        }
        else if (choice  == 1)
        {
            printf("void delay_%s()\n{\n", (choice_m == 0)? "OV" : "CTC");
            printf("\t// using Timer%d to generate time delay: %s\n", choice, print_unit(time_delay));
            //TCNT
            //Getting the initial value if in OV mode.
            if (choice_m == 0)
                value_num = 0xFFFF - value_num + 1;

            printf("\tTCNT1H = %s;\n", (choice_m == 1)? "0x0" : print_hexa(((value_num & 0xFF00) >> 8)));
            printf("\tTCNT1L = %s;\n", (choice_m == 1)? "0x0" : print_hexa((value_num & 0x00FF)));
            //CTC
            if (choice_m == 1)
            {
                printf("\tOCR1AH = %s;\n", print_hexa(((value_num & 0xFF00) >> 8)));
                printf("\tOCR1AL = %s;\n", print_hexa((value_num & 0x00FF) - 1 ));
            }
            printf("\tTCCR1A = 0;\n");
            printf("\tTCCR1B = 0b%s;\n", print_binary(reg_c));
            printf("\twhile((TIFR & (1<<%s)) == 0);\n", (choice_m == 1)? "OCF1A" : "TOV1");
            printf("\tTIFR |= (1<<%s);\n", (choice_m == 1)? "OCF1A" : "TOV1");
            printf("\tTCCR1A=0;\n");
            printf("\tTCCR1B=0;\n");
            printf("}\n");
        }

        do
        {
            fflush(stdin);
            printf("\nDo you want to run the program again? (y, n): ");
        }
        while(!scanf("%c", &ch) || (tolower(ch) != 'y'&& tolower(ch) != 'n') );
        if (ch == 'n')
            return 0;
    }




    return 0;
}

//Prints the number as integer followed with the unit second.
char* print_unit(float b)
{
    char *ret = malloc(sizeof(char) * 10);
    //\xE6 is micro symbol
    char *unit[] = {"s","ms", "\xE6s", "ns", "ps"};
    uint8_t i = 0;
    while (b < .9 && i < 5)
    {
        b *= 1000.0;
        i++;
    }
    sprintf(ret, "%0.0f%s", b, unit[i]);
    return ret;
}
//Prints the decimal number into binary.
char* print_binary( uint8_t a)
{
    char *ret = malloc(sizeof(char) * 9);
    uint8_t b = (1 << 7);
    for (int x = 0; x < 8;x ++)
    {
       ret[x] = ((a & b)? '1' : '0');
        b >>= 1;
    }
    ret[8] = '\0';
    return ret;
}
//Prints the decimal number into hexa.
char* print_hexa(uint8_t num)
{
    char *ret = malloc(sizeof(char) * 10);
    char n[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D','E','F'};
    sprintf(ret, "0x%c%c", n[(num & 0xF0) >> 4], n[num & 0x0F] );
    return ret;
}




