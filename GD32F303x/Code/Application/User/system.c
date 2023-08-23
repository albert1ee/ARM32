#include "main.h"

void GpioInit(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_AF);
	
	//KEY PA0
	gpio_init(GPIOA, GPIO_MODE_IPU,GPIO_OSPEED_10MHZ, GPIO_PIN_0);
	//BT_RX	PA2
	//BT_TX	PA3
	//SPI_NSS	PA4
	//SPI_SCK	PA5
	//SPI_MISO	PA6
	//SPI_MOSI	PA7
	//BT_INT	PB0
	//BT_WKUP	PB1
	gpio_bit_reset(GPIOB,GPIO_PIN_1);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_1);
	//BT_PDN	PB2
	gpio_bit_reset(GPIOB,GPIO_PIN_2);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
	//R_TX2	PB10
	//L_TX3	PA9
	//AD_Vmotor	PB12
	//VBT_control	PB13
	gpio_bit_set(GPIOB,GPIO_PIN_13);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_13);
	//PW_ONOFF	PB14
	gpio_bit_reset(GPIOB,GPIO_PIN_14);
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ,GPIO_PIN_14);
	//LED	PA15
	gpio_bit_reset(GPIOA, GPIO_PIN_15);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_15);
	
}

void GpioUartGpioInit(uint32_t rxport,uint32_t rxpin,uint32_t txport,uint32_t txpin)
{
	/* enable the key user clock */
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_AF);
	//Tx
	gpio_init(txport, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, txpin);
	gpio_bit_set(txport, txpin);
	//Rx
	gpio_init(rxport,GPIO_MODE_IPU,GPIO_OSPEED_10MHZ,rxpin);
	if (rxpin == GPIO_PIN_15)
	{
		nvic_irq_enable(EXTI10_15_IRQn,2U,0U);
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB,GPIO_PIN_SOURCE_15);
		exti_init(EXTI_15, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		exti_interrupt_flag_clear(EXTI_15);
	}
	else if (rxpin == GPIO_PIN_2)
	{
		nvic_irq_enable(EXTI2_IRQn,2U,0U);
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB,GPIO_PIN_SOURCE_2);
		exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		exti_interrupt_flag_clear(EXTI_2);
	}
	else if (rxpin == GPIO_PIN_11)
	{
		nvic_irq_enable(EXTI10_15_IRQn,2U,0U);
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA,GPIO_PIN_SOURCE_11);
		exti_init(EXTI_11, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		exti_interrupt_flag_clear(EXTI_11);
	}
}
void GpioUartGpioDeInit(uint32_t rxport,uint32_t rxpin,uint32_t txport,uint32_t txpin)
{
}

void UsartIrqInit(uint32_t uart,uint32_t br,int tx_enable,int rx_enable)
{
	if ((tx_enable == 0) && (rx_enable == 0))
		return;
	/* enable USART clock */
	if (uart == USART0)
	{
		gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
		rcu_periph_clock_enable(RCU_USART0);
	}
	else if (uart == USART1)
	{
		gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
		gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
		rcu_periph_clock_enable(RCU_USART1);
	}
	else if (uart == USART2)
	{
		gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_11);
		rcu_periph_clock_enable(RCU_USART2);
	}
	else if (uart == UART3)
	{
		gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
		gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_11);
		rcu_periph_clock_enable(RCU_UART3);
	}
	else if (uart == UART4)
	{
		gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_12);
		gpio_init(GPIOD, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
		rcu_periph_clock_enable(RCU_UART4);
	}
	/* USART configure */
	usart_deinit(uart);
	usart_baudrate_set(uart, br);
//	usart_word_length_set(uart,USART_WL_8BIT);
//	usart_stop_bit_set(uart,USART_STB_1BIT);
//	usart_parity_config(uart,USART_PM_NONE);
	if (rx_enable)
	usart_receive_config(uart, USART_RECEIVE_ENABLE);
	if (tx_enable)
	usart_transmit_config(uart, USART_TRANSMIT_ENABLE);
	usart_enable(uart);
	usart_interrupt_enable(uart, USART_INT_RBNE);
	if (uart == USART0)
	{
		nvic_irq_enable(USART0_IRQn, 0, 0);
	}
	else if (uart == USART1)
	{
		nvic_irq_enable(USART1_IRQn, 0, 1);
	}
	else if (uart == USART2)
	{
		nvic_irq_enable(USART2_IRQn, 0, 2);
	}
	else if (uart == UART3)
	{
		nvic_irq_enable(UART3_IRQn, 0, 3);
	}
	else if (uart == UART4)
	{
		nvic_irq_enable(UART4_IRQn, 0, 4);
	}
}
void UsartIrqDeInit(uint32_t uart)
{
	usart_disable(uart);
	if (uart == USART0)
	{
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
		nvic_irq_disable(USART0_IRQn);
	}
	else if (uart == USART1)
	{
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
		gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_3);
		nvic_irq_disable(USART1_IRQn);
	}
	else if (uart == USART2)
	{
		gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
		gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_11);
		nvic_irq_disable(USART2_IRQn);
	}
	else if (uart == UART3)
	{
		gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
		gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_11);
		nvic_irq_disable(UART3_IRQn);
	}
	else if (uart == UART4)
	{
		gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_12);
		gpio_init(GPIOD, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
		nvic_irq_disable(UART4_IRQn);
	}
}



uint32_t GetUartPortByName(char * uart)
{
	if (memcmp(uart,"USART0",strlen("USART0")) == 0)
	{
		return USART0;
	}
	else if (memcmp(uart,"USART1",strlen("USART1")) == 0)
	{
		return USART1;
	}
	else if (memcmp(uart,"USART2",strlen("USART2")) == 0)
	{
		return USART2;
	}
	else if (memcmp(uart,"UART3",strlen("UART3")) == 0)
	{
		return UART3;
	}
	else if (memcmp(uart,"UART4",strlen("UART4")) == 0)
	{
		return UART4;
	}
	else
	{
		rt_kprintf("uart search fail by %s\n",uart);
	}
	return 0;
}


//52us定时器，用于定时发送从机数据
void Timer0Init(void)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);

    /* TIMER0 configuration */
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 6240-1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);
	
    timer_interrupt_enable(TIMER0,TIMER_INT_UP);

    /* TIMER0 counter enable */
    timer_enable(TIMER0);

	nvic_irq_enable(TIMER0_UP_IRQn, 1, 1);
}

