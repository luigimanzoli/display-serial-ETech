#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

// Bibliotecas referentes à configuração do display
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

// Arquivo .pio
#include "display.pio.h"

// Número de LEDs da matriz
#define NUM_PIXELS 25

// Pino de saída da matriz
#define OUT_PIN 7

// Definição dos LEDs RGB
#define RLED_PIN 13
#define GLED_PIN 11
#define BLED_PIN 12

// Definição dos botões
#define BTNA_PIN 5
#define BTNB_PIN 6
#define BTNJ_PIN 22 // Botão do Joystick

// Variável de mudança de string para inteiro
static volatile int ic = 0;

// Variável de controle do terceiro botão
static volatile int cont = 0;

// Variável ligada ao debounce dos botões
static volatile uint32_t last_time = 0; 

// Inicializa a estrutura do display
ssd1306_t ssd; 

// Inicialização dos lEDs e Botões
void init_all() {
    gpio_init(RLED_PIN);
    gpio_set_dir(RLED_PIN, GPIO_OUT);
    gpio_put(RLED_PIN, 0);

    gpio_init(GLED_PIN);
    gpio_set_dir(GLED_PIN, GPIO_OUT);
    gpio_put(GLED_PIN, 0);

    gpio_init(BLED_PIN);
    gpio_set_dir(BLED_PIN, GPIO_OUT);
    gpio_put(BLED_PIN, 0);

    gpio_init(BTNA_PIN);
    gpio_set_dir(BTNA_PIN, GPIO_IN);
    gpio_pull_up(BTNA_PIN);

    gpio_init(BTNB_PIN);
    gpio_set_dir(BTNB_PIN, GPIO_IN);
    gpio_pull_up(BTNB_PIN);

    gpio_init(BTNJ_PIN);
    gpio_set_dir(BTNJ_PIN, GPIO_IN);
    gpio_pull_up(BTNJ_PIN);
}

// Matriz com todos os dígitos
double digits[10][25] = {
{ // Digito 0
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 1
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 2
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 3
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 4
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0
},
{ // Digito 5
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 6
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 7
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0
},
{ // Digito 8
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0
},
{ // Digito 9
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 1.0, 0.0,
    0.0, 1.0, 1.0, 1.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0
}

};

// Rotina para definição da intensidade de cores do LED
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

// Inicializa o sistema de clock
void clock_init() {
    bool ok = set_sys_clock_khz(100000, false);
    if (ok) {
        printf("Clock setado para %ld Hz\n", clock_get_hz(clk_sys));
    } else {
        printf("Falha ao configurar o clock\n");
    }
}

// Configura a PIO
void pio_config(PIO pio, uint *offset, uint *sm) {
    *offset = pio_add_program(pio, &display_program);
    *sm = pio_claim_unused_sm(pio, true);
    display_program_init(pio, *sm, *offset, OUT_PIN);
}

// Função para imprimir um número na matriz de LEDs
void print_digit(int digit, PIO pio, uint sm, double r, double g, double b){
    // Valor para intensidade dos LEDs
    double ity = 0.01;

    // Iniciando a variável que detém informação das cores de cada LED da matriz
    uint32_t led_value;

    // Condição para que os valores não ultrapassem o intervalor desejado
    if (digit <= 9 && digit >= 0){
        for (int16_t i = 0; i < NUM_PIXELS; i++) {
            // Define a cor dos LEDs baseados nos valores de r, g e b
            led_value = matrix_rgb(b*ity*(digits[digit][24 - i]), r*ity*(digits[digit][24 - i]), g*ity*(digits[digit][24 - i]));
            pio_sm_put_blocking(pio, sm, led_value); // Envia o valor para o LED
        }
    } else if (digit < 0) {
        printf("Valor incompatível.\n");
    } else if (digit > 9){
        printf("Valor incompatível.\n");
    }
}

// Inicialização do PIO e das variáveis necessárias
PIO pio = pio0;
uint32_t led_value;
uint offset, sm;

// Função que é chamada quando ocorre a interrupção
void gpio_irq_handler(uint gpio, uint32_t events){

    // Definição da variável do tempo atual do sistema dês do começo
    uint32_t current_time = to_us_since_boot(get_absolute_time());
        if (current_time - last_time > 200000){
            last_time = current_time;
            
            if (gpio == BTNA_PIN){

                gpio_put(GLED_PIN, !gpio_get(GLED_PIN)); // Alterna o estado do LED verde
                printf("Estado do LED Verde Alternado.\n");

                if (gpio_get(GLED_PIN)){
                    ssd1306_draw_string(&ssd, "LED Verde", 2, 8); // Desenha uma string 
                    ssd1306_draw_string(&ssd, "         ", 2, 21);
                    ssd1306_draw_string(&ssd, "LIGADO", 2, 21);
                }else{
                    ssd1306_draw_string(&ssd, "LED Verde", 2, 8); // Desenha uma string 
                    ssd1306_draw_string(&ssd, "DESLIGADO", 2, 21);
                }
                
                ssd1306_send_data(&ssd); // Atualiza o display  

            }
            else if (gpio == BTNB_PIN){

                gpio_put(BLED_PIN, !gpio_get(BLED_PIN)); // Alterna o estado do LED azul
                printf("Estado do LED Azul Alternado.\n");

                if (gpio_get(BLED_PIN)){
                    ssd1306_draw_string(&ssd, "LED Azul", 2, 38); // Desenha uma string
                    ssd1306_draw_string(&ssd, "         ", 2, 51); 
                    ssd1306_draw_string(&ssd, "LIGADO", 2, 51);
                }else{
                    ssd1306_draw_string(&ssd, "LED Azul", 2, 38); // Desenha uma string 
                    ssd1306_draw_string(&ssd, "DESLIGADO", 2, 51);
                }
                ssd1306_send_data(&ssd); // Atualiza o display

            }
            else if (gpio == BTNJ_PIN){
                cont++;
                if (cont % 2 == 0){
                    ssd1306_fill(&ssd, false); // Limpa o display
                    ssd1306_send_data(&ssd); // Manda a informação para o display
                }else{
                    ssd1306_fill(&ssd, false); // Limpa o display
                    ssd1306_draw_string(&ssd, "Alfabeto", 2, 10);
                    ssd1306_draw_string(&ssd, "minusculo", 2, 19);
                    ssd1306_draw_string(&ssd, "abcdefghijklmnopqrstuvwxyz", 2, 38);
                    ssd1306_send_data(&ssd); // Manda a informação para o display
                }
                
            }
    }

}

// Função principal
int main() {
    // Inicializa clock, stdio e configurações
    stdio_init_all();
    init_all();
    clock_init();

    pio_config(pio, &offset, &sm);

    printf("Sistema inicializado.\n");

    // Configuração dos botões como interrupções
    gpio_set_irq_enabled_with_callback(BTNA_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTNB_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTNJ_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    while (true) {
        
       if (stdio_usb_connected())
        { // Certifica-se de que o USB está conectado
            char c;
            
            if (scanf("%c", &c) == 1) // Lê caractere da entrada padrão
            { 
                printf("Recebido: '%c'\n", c);

                ssd1306_fill(&ssd, false); // Limpa o display
                ssd1306_draw_string(&ssd, "DIGITADO:", 35, 28);
                ssd1306_draw_char(&ssd, c, 60, 38); // Desenha o caractere digitado
                ssd1306_send_data(&ssd); // Manda a informação para o display

                if (c >= '0' && c <= '9'){ // Se o caractere digitado for um número de 0 a 9, imprime na matriz
                    ic = c - '0'; // Conversão de string para inteiro
                    print_digit(ic, pio, sm, 1, 1, 1);
                }
            }
        }
        sleep_ms(1000);
    }
    return 0;

}
