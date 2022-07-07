#define F_CPU 16000000UL
#include
#include
#include
#include

volatile unsigned int adc_data[8] = {0,0,0,0,0,0,0,0};
volatile unsigned int adc_data_max[8] = {0,0,0,0,0,0,0,0};
volatile unsigned int adc_data_min[8] = {1023,1023,1023,1023,1023,1023,1023,1023};
volatile float normalization[8] = {0,0,0,0,0,0,0,0};
volatile int mode = 0;
volatile int data = 0;
volatile int i = 0;
volatile int weight[8] = {20,19,9,5,-5,-9,-19,-20};
volatile int result = 0;
volatile int lspeed = 0;
volatile int rspeed = 0;
volatile int previousweight = 0;
volatile int line[8] = {0,};
volatile int black = 0;
volatile int led = 0;
volatile int semiblack = 0;
volatile int cross;
volatile int cnt;
volatile int pass;
volatile int cnt2;

int adc_SetChannel(unsigned char);


ISR(INT0_vect)
{
	mode = 1;//ADC값 받기
}

ISR(INT1_vect)
{
	mode = 2;//정규화 후 주행
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 100; //10ms



	//adc값 받기
	for (int i = 0; i < 8; i++)
	{
		adc_data[i] = adc_SetChannel(i);
	}
	//만약 모드가 1이면 adc값 최대, 최소 받기.
	if(mode == 1)
	{
		for (int i = 0; i < 8; i++)
		{
			if(adc_data[i] > adc_data_max[i])
			adc_data_max[i] = adc_data[i];
			if(adc_data[i] < adc_data_min[i])
			adc_data_min[i] = adc_data[i];
		}
	}
	//만약 모드가 2이면 정규화 시킨 후 주행
	if(mode == 2)
	{
		cnt++;
		TCNT0 = 100; //10ms

		cnt2++;

		led = 0;
		black = 0;

		for(i = 0; i < 8; i++)
		{
			normalization[i] = (((float)adc_data[i]-(float)adc_data_min[i]) * 100) / ((float)adc_data_max[i] - (float)adc_data_min[i]) ;
			if(normalization[i] < 35)
			{
				line[i] = 1;
				black++;

			}
			else
			{
				line[i] = 0;
			}
		}
		if(pass == 1 && line[1] == 0 && line[6] == 0)
		{
			if(cnt2 > 7300)
			cross++;
			pass = 0;

		}
		else if(line[1] == 1 && line [6] == 1)
		{
			pass = 1;
		}

		if(cross == 1)
		{
			PORTA = 0b11111110;
			PORTE = 0b00001001;
			OCR1A = 700;
			OCR1B = 700;
		}

		if(cross == 2)
		{
			PORTA = 0b11111100;
			PORTE = 0b00001001;
			OCR1A = 615;
			OCR1B = 615;

		}

		if(cross == 3)
		{
			PORTA = 0b11111000;
			PORTE = 0b00000101;
			OCR1A = 600;
			OCR1B = 600;
			_delay_ms(500);
			cross++;
		}

		if(cross == 6)
		{
			PORTA = 0b11110000;
			PORTE = 0b00001001;
			OCR1A = 0;
			OCR1B = 0;
			_delay_ms(3000);
			cross++;
		}


		if(cross == 8)
		{
			PORTA = 0b11100000;
			PORTE = 0b00000101;
			OCR1A = 600;
			OCR1B = 600;
			_delay_ms(1500);
			cross++;
		}



		if(cross == 11)
		{
			PORTA = 0b10000000;
			PORTE = 0b00001001;
			OCR1A = 0;
			OCR1B = 0;
			_delay_ms(5000);
		}

		if(black == 8)
		{
			PORTE = 0b00001001;
			OCR1A = 600;
			OCR1B = 600;
		}

		if(black == 0)
		{
			if(result < 0)
			{
				PORTE = 0b00000101; //0000 1001 전진방향
				OCR1A = 650;
				OCR1B = 650;

			}
			else if(result > 0)
			{
				PORTE = 0b00001010;
				OCR1A = 650;
				OCR1B = 650;

			}
		}
		else
		{
			result = 0;
			PORTE = 0b00001001;
			for(i = 0; i < 8; i++)
			{
				result += normalization[i]*weight[i];
			}

			rspeed = 615 - (result/15);
			if (rspeed>799 ) rspeed = 790;
			else if (rspeed< 10) rspeed = 0;

			lspeed = 615 + (result/15);
			if (lspeed>799 ) lspeed = 790;
			else if (lspeed< 10) lspeed = 0;


			OCR1A = rspeed; //오른쪽
			OCR1B = lspeed; //왼쪽

			//예외처리
			previousweight = result;

		}

	}

}

int main()
{
	//포트설정
	DDRA = 0xff;
	PORTA = 0xff;
	DDRF = 0x00;
	DDRD = 0b00001000;
	ADMUX = 0b10000000;
	ADCSRA = 0b10000111;

	//motor
	DDRB = 0xff;
	DDRE = 0x0f;
	PORTE = 0b00001001;
	TCCR1A = (1 TCCR1B = (1 ICR1 = 799;
	OCR1A = 0;
	OCR1B = 0;

	//Uart
	UBRR1H = 0;
	UBRR1L = 8; //baud rate : 115200
	UCSR1B = (1
	//스위치
	EICRA = (1 EIMSK = (1
	//타이머
	TCCR0 = (1 TIMSK = (1 TCNT0 = 100;

	sei();
	while (1);
}

int adc_SetChannel(unsigned char Adc_input)
{
	ADMUX = (Adc_input | 0x40);
	ADCSRA |= (1 while(!(ADCSRA & (1 return ADC;
}

