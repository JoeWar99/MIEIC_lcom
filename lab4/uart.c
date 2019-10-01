#include <lcom/lcf.h>
#include "uart_macros.h"
#include "fila.h"
#include "i8042.h"
#include "i8254.h"
#include "uart.h"

static int uart_hook_id = 4;

fila *fila_transmitter;
fila *fila_receiver;

fila *get_transmitter()
{
  return fila_transmitter;
}
fila *get_receiver()
{
  return fila_receiver;
}

int serial_set(unsigned long bits, unsigned long stop, long parity, unsigned long rate)
{
  uint32_t lcr;
  if (sys_inb(LINE_CONTROL_REG, &lcr))
    return 1;

  lcr &= BIT(UART_REGISTER_LCR_BREAK_CONTROL_BIT);
  lcr |= BIT(UART_REGISTER_LCR_DLAB_BIT); // Set to 1 because we are going to change the rate

  // Set the bits per char in an efficient way
  bits -= 5;
  if (bits > 3)
    return 1;
  lcr |= bits << UART_REGISTER_LCR_BITS_PER_CHAR_BIT;
  
  if (stop == 2)
  {
    lcr |= BIT(UART_REGISTER_LCR_NUM_STOP_BITS_BIT);
  }

  switch (parity)
  {
  case -1:
    break;
  case 0:
    lcr |= (3 << UART_REGISTER_LCR_PARITY_CONTROL_BIT);
    break;
    //11
  case 1:
    lcr |= (1 << UART_REGISTER_LCR_PARITY_CONTROL_BIT);
    break;
    //1
  default:
    return 1;
  }
  if (sys_outb(LINE_CONTROL_REG, lcr))
  {
    return 1;
  }

  rate = UART_MAX_BITRATE / rate;
  if (sys_outb(RECEIVER_BUFFER_REG, WORD_LSB(rate)))
  {
    return 1;
  }

  if (sys_outb(INTERRUPT_ENABLE_REGISTER, WORD_MSB(rate)))
  {
    return 1;
  }
  if (sys_outb(LINE_CONTROL_REG, lcr & ~BIT(UART_REGISTER_LCR_DLAB_BIT)))
  {
    return 1;
  }

  return 0;
}

int transmit_queue()
{

  int contador_bytes = 0;
  uint32_t aux_sender = INITIALIZE0;
  uint32_t leitura;
  unsigned char *carater;

  while ((fila_tamanho(fila_transmitter) > 0))
  {

    sys_inb(LINE_STATUS_REG, &leitura);

    if ((leitura & BIT(5)) != 0)
    {
      carater = fila_front(fila_transmitter);

      if (carater == NULL)
      {
        printf("Char Invalido\n");
        return -1;
      }

      aux_sender = *carater;
      sys_outb(TRANSMITTER_HOLDING_REG, aux_sender);
      printf("character enviado %c \n", *carater);
      fila_pop(fila_transmitter);
      contador_bytes++;
    }
  }
  return 0;
}

int receive_queue()
{
  uint32_t leitura_aux = INITIALIZE0;
  unsigned char *aux = (unsigned char *)malloc(sizeof(unsigned char));
  int contador_bytes = 0;
  uint32_t aux_32;

  sys_inb(RECEIVER_BUFFER_REG, &leitura_aux);

  while (leitura_aux & BIT(UART_REGISTER_LSR_RECEIVER_DATA_BIT))
  {
    if (aux == NULL)
    {
      return -1;
    }
    if (leitura_aux & (BIT(UART_REGISTER_LSR_OVERRUN_ERROR_BIT) | BIT(UART_REGISTER_LSR_PARITY_ERROR_BIT) | BIT(UART_REGISTER_LSR_FRAMING_ERROR_BIT)))
    {
      return -1;
    }
    sys_inb(RECEIVER_BUFFER_REG, &aux_32);

    *aux = (unsigned char)aux_32;

    sys_inb(RECEIVER_BUFFER_REG, &leitura_aux);
    printf("character recebido %c \n", *aux);
    fila_push(fila_receiver, aux);
    contador_bytes++;
  }

  return 0;
}

void self_clearing_receiver()
{
  uint32_t leitura;
  sys_inb(FIFO_CONTROL_REG, &leitura);
  leitura = leitura | BIT(1);
  sys_outb(FIFO_CONTROL_REG, leitura);

  return;
}

void self_clearing_transmitter()
{
  uint32_t leitura;

  sys_inb(FIFO_CONTROL_REG, &leitura);

  leitura = leitura | BIT(2);
  sys_outb(FIFO_CONTROL_REG, leitura);
}
int uart_subscribe_int(uint8_t *bit_no)
{
  uint32_t leitura;
  if (bit_no == NULL)
  { //erro caso a apontador bit no não seja valido
    return -1;
  }
  *bit_no = (uint8_t)uart_hook_id; // bit no recebe o hook id que nos configuramos para mandarmos para o kernel
  if (sys_irqsetpolicy(IRQ_UART_LINE, IRQ_REENABLE | IRQ_EXCLUSIVE, &uart_hook_id) != OK)
  { //da set a policy
    printf("ERRO , sys_irqsetpolicy falhou\n");
    return -1;
  }
  sys_outb(FIFO_CONTROL_REG, BIT(UART_REGISTER_FCR_ENABLE_FIFO) |
                                 BIT(UART_REGISTER_FCR_CLEAR_RECEIVE_FIFO) |
                                 BIT(UART_REGISTER_FCR_CLEAR_TRANSMIT_FIFO));

  sys_outb(INTERRUPT_ENABLE_REGISTER,
           BIT(UART_REGISTER_IER_RECEIVED_DATA_INT) |
               BIT(UART_REGISTER_IER_TRANSMITTER_EMPTY_INT) |
               BIT(UART_REGISTER_IER_RECEIVER_LSR_INT));

  sys_inb(INTERRUPT_ID_REG, &leitura);
  if ((leitura & 0xC0) == 0)
  {
    printf("Erro a inicializar o FIFO\n");
    return -1;
  }
  else
  {
    printf("Tudo ok, FIFO Working\n");
  }

  fila_transmitter = fila_nova();
  fila_receiver = fila_nova();
  return 0;
}

int uart_ih()
{

  uint32_t leitura;
  uint32_t leitura_line_status;
  sys_inb(INTERRUPT_ID_REG, &leitura);
  //int j=0;
  //  unsigned char *aux=(unsigned char*)malloc(sizeof(unsigned char));
  while (!(leitura & BIT(UART_REGISTER_IIR_INTERRUPT_STATUS_BIT)))
  {

    leitura = leitura & (BIT(3) | BIT(2) | BIT(1));
    leitura = leitura >> 1;

    switch (leitura)
    {
    case 0:
      printf("MODEM - ERRO\n");
      uint32_t msr;
      if (sys_inb(MODEM_STATUS_REG, &msr))
        return 1;
      break;

    case 1:
      if (transmit_queue() == -1)
      {
        return -1;
      }
      break;

    case 2:
      if (receive_queue() == -1)
      {
        return -1;
      }
      break;

    case 3:
      sys_inb(LINE_STATUS_REG, &leitura_line_status);
      if (leitura & (BIT(1) | BIT(2) | BIT(3)))
      {
        printf("ERRO, overrun error or parity error or framing error\n");
        return -1;
      }
      break;

    case 6:
      if (receive_queue() == -1)
      {
        return -1;
      }
      break;
    }
    sys_inb(INTERRUPT_ID_REG, &leitura);
  }
  return 0;
}

int uart_unsubscribe_int()
{

  uint32_t leitura;

  if (sys_irqdisable(&uart_hook_id) != OK)
  { //disable primeiro, para depois tirar a policy set
    printf("ERRO , sys_irqdisable falhou\n");
    return -1;
  }
  //------------- Da Disable no HW ao hardware

  sys_inb(INTERRUPT_ENABLE_REGISTER, &leitura);

  leitura = leitura & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3));

  sys_outb(INTERRUPT_ENABLE_REGISTER, leitura);

  //-----------Da disable aos FIFOS

  sys_inb(FIFO_CONTROL_REG, &leitura);

  leitura = leitura & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3));

  sys_outb(FIFO_CONTROL_REG, leitura);

  //Falta chamar a func clear

  self_clearing_receiver();
  self_clearing_transmitter();

  if (sys_irqrmpolicy(&uart_hook_id) != OK)
  { //disbable nos interrupts
    printf("ERRO , sys_irqrmpolicy falhou\n");
    return -1;
  }

  return 0;
}
