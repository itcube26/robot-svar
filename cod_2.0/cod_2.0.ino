// ============================================================================
// РОБОТ-СВАРЩИК С УПРАВЛЕНИЕМ ПО BLUETOOTH
// ============================================================================

// --- Конфигурация пинов ---
#define M1_dir     45   // Направление двигателя 1
#define M1_Speed   44   // Скорость двигателя 1 (ШИМ)
#define M2_dir     47   // Направление двигателя 2 (ВНИМАНИЕ: проверьте поддержку ШИМ на вашем контроллере)
#define M2_Speed   46   // Скорость двигателя 2 (ШИМ)
#define LASER_PIN  9    // Пин управления сваркой (лазером)

// --- Глобальные переменные состояния ---
int currentSpeed = 0;       // Текущая скорость (0-255)
bool isWelding = false;     // Состояние сварочного модуля

// ============================================================================
void setup() {
  // Инициализация портов связи
  Serial.begin(9600);       // Для отладки в мониторе порта
  Serial2.begin(115200);    // Для Bluetooth модуля (HC-05/HC-06)

  // Настройка пинов двигателей и сварки на выход
  pinMode(M1_dir, OUTPUT);
  pinMode(M1_Speed, OUTPUT);
  pinMode(M2_dir, OUTPUT);
  pinMode(M2_Speed, OUTPUT);
  pinMode(LASER_PIN, OUTPUT);

  // Начальное состояние: остановка и выключенная сварка
  stopMotors();
  setWelder(false);
  
  Serial.println("Robot Welder Ready. Waiting for BT commands...");
}

// ============================================================================
void loop() {
  // Обработка входящих команд от Bluetooth пульта
  if (Serial2.available()) {
    char cmd = Serial2.read();
    processBTCommand(cmd);
  }
}

// ============================================================================
// Обработчик команд Bluetooth (на основе строки: xXYYFSBSLSRS123456789876543210)
// ============================================================================
void processBTCommand(char cmd) {
  switch (cmd) {
    // --- Управление скоростью (цифры 1-9) ---
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      currentSpeed = map(cmd - '0', 1, 9, 50, 255); // Масштабируем 1-9 в 50-255 PWM
      Serial.print("Speed set to: "); 
      Serial.println(currentSpeed);
      break;

    // --- Управление движением ---
    case 'F': // Вперед
      moveMotors(HIGH, HIGH, currentSpeed, currentSpeed);
      break;
      
    case 'B': // Назад
      moveMotors(LOW, LOW, currentSpeed, currentSpeed);
      break;
      
    case 'L': // Влево (разворот на месте)
      moveMotors(LOW, HIGH, currentSpeed, currentSpeed);
      break;
      
    case 'R': // Вправо (разворот на месте)
      moveMotors(HIGH, LOW, currentSpeed, currentSpeed);
      break;
      
    case 'S': // Стоп
    case '0': // Экстренный стоп + выключение сварки
      stopMotors();
      if (cmd == '0') {
        setWelder(false);
        Serial.println("Emergency Stop & Welder OFF");
      }
      break;

    // --- Управление сваркой (лазером) ---
    case 'x': // Включить сварку
      setWelder(true);
      break;
      
    case 'X': // Выключить сварку
      setWelder(false);
      break;

    // --- Игнорируемые команды джойстика (центровка, отпускание) ---
    case 'Y':
    case 'y':
      // Некоторые приложения отправляют 'Y' или 'y' при возврате джойстика в центр.
      // Мы их игнорируем, чтобы не сбивать текущее состояние, 
      // либо можно раскомментировать stopMotors(), если нужно останавливаться при отпускании.
      // stopMotors(); 
      break;

    default:
      // Неизвестная команда, можно игнорировать или вывести в лог
      break;
  }
}

// ============================================================================
// Вспомогательные функции управления
// ============================================================================

// Универсальная функция установки состояния двигателей
void moveMotors(bool dir1, bool dir2, int speed1, int speed2) {
  // Если скорость не была задана командами 1-9, устанавливаем безопасный минимум
  if (speed1 == 0) speed1 = 100;
  if (speed2 == 0) speed2 = 100;

  digitalWrite(M1_dir, dir1);
  digitalWrite(M2_dir, dir2);
  analogWrite(M1_Speed, speed1);
  analogWrite(M2_Speed, speed2);
}

// Остановка обоих двигателей
void stopMotors() {
  analogWrite(M1_Speed, 0);
  analogWrite(M2_Speed, 0);
  currentSpeed = 0; // Сбрасываем целевую скорость
}

// Управление сварочным модулем
void setWelder(bool state) {
  isWelding = state;
  if (isWelding) {
    analogWrite(LASER_PIN, 255); // Полная мощность сварки/лазера
    Serial.println("Welder: ON");
  } else {
    analogWrite(LASER_PIN, 0);   // Выключено
    Serial.println("Welder: OFF");
  }
}
