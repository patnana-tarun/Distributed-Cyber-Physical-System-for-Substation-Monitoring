# ═════════════════════════════════════════════════════════════
# SMART ACCESS CONTROL SYSTEM v3 - FINAL
# ═════════════════════════════════════════════════════════════

# ─────────────────────────────────────────────────────────────
# CONFIGURATION
# ─────────────────────────────────────────────────────────────
BLYNK_AUTH     = "REPLACE_WITH_BLYNK_AUTH_TOKEN"
WIFI_SSID      = "REPLACE_WITH_WIFI_SSID"
WIFI_PASS      = "REPLACE_WITH_WIFI_PASSWORD"
BLYNK_SERVER   = "blynk.cloud"
BLYNK_PORT     = 80
BLYNK_INSECURE = True

USERS = {
    "101": "REPLACE_WITH_PIN",
}

SERVO_CLOSED        = 0
SERVO_OPEN          = 90
INPUT_TIMEOUT       = 30000
GATE_OPEN_TIME      = 5000
LCD_ADDR            = 0x27
WIFI_CHECK_INTERVAL = 60000

# ─────────────────────────────────────────────────────────────
# IMPORTS
# ─────────────────────────────────────────────────────────────
import network
import time
from machine import Pin, PWM, I2C, UART

blynk = None
BLYNK_AVAILABLE = False
try:
    import blynklib
    BLYNK_AVAILABLE = True
except ImportError:
    print("Warning: blynklib not found.")

# ─────────────────────────────────────────────────────────────
# HARDWARE
# ─────────────────────────────────────────────────────────────

servo_pwm = PWM(Pin(15))
servo_pwm.freq(50)
current_servo_angle = SERVO_CLOSED
gate_is_open        = False

def set_servo(angle):
    global current_servo_angle
    duty = int(1638 + (angle / 180) * (8192 - 1638))
    servo_pwm.duty_u16(duty)
    current_servo_angle = angle

pir     = Pin(16, Pin.IN)
RELAY_1 = Pin(17, Pin.OUT)
RELAY_2 = Pin(18, Pin.OUT)
RELAY_3 = Pin(19, Pin.OUT)
RELAY_4 = Pin(20, Pin.OUT)
relays  = [RELAY_1, RELAY_2, RELAY_3, RELAY_4]
for r in relays:
    r.value(0)

i2c = I2C(1, sda=Pin(2), scl=Pin(3), freq=400000)
bt  = UART(0, baudrate=9600, tx=Pin(0), rx=Pin(1))

KEY_MAP  = [['1','2','3','A'],['4','5','6','B'],['7','8','9','C'],['*','0','#','D']]
row_pins = [Pin(p, Pin.OUT)               for p in [10, 11, 12, 13]]
col_pins = [Pin(p, Pin.IN, Pin.PULL_DOWN) for p in [6, 7, 21, 22]]
led      = Pin("LED", Pin.OUT)

# ─────────────────────────────────────────────────────────────
# LCD
# ─────────────────────────────────────────────────────────────

def lcd_send_byte(data):
    i2c.writeto(LCD_ADDR, bytes([data]))

def lcd_pulse_enable(data):
    lcd_send_byte(data | 0x04); time.sleep_us(1)
    lcd_send_byte(data & ~0x04); time.sleep_us(50)

def lcd_write_nibble(nibble, mode=0):
    lcd_pulse_enable((nibble & 0xF0) | 0x08 | mode)

def lcd_write_byte(byte, mode=0):
    lcd_write_nibble(byte & 0xF0, mode)
    lcd_write_nibble((byte << 4) & 0xF0, mode)

def lcd_cmd(cmd):
    lcd_write_byte(cmd, 0); time.sleep_ms(2)

def lcd_char(ch):
    lcd_write_byte(ord(ch), 1)

def lcd_init():
    time.sleep_ms(50)
    for v in [0x30, 0x30, 0x30, 0x20]:
        lcd_write_nibble(v); time.sleep_ms(5)
    for cmd in [0x28, 0x0C, 0x06, 0x01]:
        lcd_cmd(cmd)
    time.sleep_ms(5)

def lcd_clear():
    lcd_cmd(0x01); time.sleep_ms(5)

def lcd_set_cursor(col, row):
    lcd_cmd(0x80 | (col + [0x00, 0x40][row]))

def lcd_print(text):
    for ch in text: lcd_char(ch)

def lcd_show(row0="", row1=""):
    lcd_clear()
    lcd_set_cursor(0, 0); lcd_print(row0[:16])
    lcd_set_cursor(0, 1); lcd_print(row1[:16])
    print(f"LCD: [{row0}] [{row1}]")

# ─────────────────────────────────────────────────────────────
# KEYPAD
# ─────────────────────────────────────────────────────────────

def scan_keypad():
    for r_idx, row in enumerate(row_pins):
        row.value(1)
        for c_idx, col in enumerate(col_pins):
            if col.value() == 1:
                time.sleep_ms(20)
                if col.value() == 1:
                    key = KEY_MAP[r_idx][c_idx]
                    while col.value() == 1: pass
                    row.value(0)
                    return key
        row.value(0)
    return None

# ─────────────────────────────────────────────────────────────
# WIFI
# ─────────────────────────────────────────────────────────────

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        lcd_show("Connecting WiFi", "Please wait...")
        wlan.connect(WIFI_SSID, WIFI_PASS)
        timeout = time.time() + 20
        while not wlan.isconnected() and time.time() < timeout:
            led.toggle(); time.sleep_ms(200)
        led.value(0)
    if wlan.isconnected():
        ip = wlan.ifconfig()[0]
        print(f"WiFi OK: {ip}")
        lcd_show("WiFi Connected!", ip)
        time.sleep(1)
        return wlan
    else:
        lcd_show("WiFi FAILED!", "Offline mode")
        time.sleep(2)
        return None

wlan = connect_wifi()

# ─────────────────────────────────────────────────────────────
# BLYNK
# ─────────────────────────────────────────────────────────────

if BLYNK_AVAILABLE and wlan and wlan.isconnected():
    try:
        blynk = blynklib.Blynk(
            BLYNK_AUTH,
            server=BLYNK_SERVER,
            port=BLYNK_PORT,
            insecure=BLYNK_INSECURE
        )
        print("Blynk ready")
    except Exception as e:
        print(f"Blynk init error: {e}")
        blynk = None

def blynk_write_safe(pin, value):
    if blynk:
        try: blynk.virtual_write(pin, value)
        except: pass

# ─────────────────────────────────────────────────────────────
# STATE + PERSISTENCE
# ─────────────────────────────────────────────────────────────

people_inside   = 0
users_inside    = set()
access_logs     = []
start_time      = time.time()
screen          = "HOME"
input_id        = ""
input_pw        = ""
input_timeout   = 0
delayed_actions = []
last_pir        = 0
last_pir_time   = 0
last_wifi_check = 0   # ← module-level, no global needed inside loop

def save_state():
    try:
        with open("state.txt", "w") as f:
            f.write(str(people_inside) + "\n")
            f.write(",".join(users_inside) + "\n")
    except: pass

def load_state():
    global people_inside, users_inside
    try:
        with open("state.txt", "r") as f:
            lines = f.read().strip().split("\n")
            people_inside = int(lines[0])
            users_inside  = set(x for x in lines[1].split(",") if x) if len(lines) > 1 else set()
            people_inside = len(users_inside)
    except:
        people_inside = 0
        users_inside  = set()

load_state()
print(f"State: {people_inside} inside → {users_inside}")

# ─────────────────────────────────────────────────────────────
# UTILITIES
# ─────────────────────────────────────────────────────────────

def get_uptime():
    e = int(time.time() - start_time)
    return f"{e//3600:02d}:{(e%3600)//60:02d}:{e%60:02d}"

def add_log(msg):
    entry = f"[{get_uptime()}] {msg}"
    access_logs.append(entry)
    if len(access_logs) > 50:
        access_logs.pop(0)
    blynk_write_safe(5, entry + "\n")
    try: bt.write((entry + "\n").encode())
    except: pass
    print(entry)

def blink_led(times=1, delay_ms=100):
    for _ in range(times):
        led.value(1); time.sleep_ms(delay_ms)
        led.value(0); time.sleep_ms(delay_ms)

# ─────────────────────────────────────────────────────────────
# MULTI-SLOT SCHEDULER
# ─────────────────────────────────────────────────────────────

def schedule_action(fn, delay_ms):
    deadline = time.ticks_add(time.ticks_ms(), delay_ms)
    delayed_actions.append((deadline, fn))

def check_delayed_actions():
    now = time.ticks_ms()
    due = [(d, f) for d, f in delayed_actions if time.ticks_diff(now, d) >= 0]
    for item in due:
        delayed_actions.remove(item)
        item[1]()

def cancel_actions_of(fn):
    to_remove = [(d, f) for d, f in delayed_actions if f is fn]
    for item in to_remove:
        delayed_actions.remove(item)

# ─────────────────────────────────────────────────────────────
# SCREENS
# ─────────────────────────────────────────────────────────────

def go_home():
    global screen, input_id, input_pw, input_timeout
    screen = "HOME"; input_id = ""; input_pw = ""; input_timeout = 0
    lcd_show("A:Entry  B:Exit", "C:Enter  D:Prev")

def show_entry_id():
    global screen
    screen = "ENTRY_ID"
    lcd_show("ENTRY - UserID:", input_id + "_")

def show_entry_pw():
    global screen
    screen = "ENTRY_PW"
    lcd_show("ENTRY - Password", "*" * len(input_pw) + "_")

def show_exit_id():
    global screen
    screen = "EXIT_ID"
    lcd_show("EXIT - UserID:", input_id + "_")

# ─────────────────────────────────────────────────────────────
# GATE — auto-close always via scheduler
# ─────────────────────────────────────────────────────────────

def open_gate(source="local"):
    global gate_is_open
    gate_is_open = True
    set_servo(SERVO_OPEN)
    blynk_write_safe(6, 1)
    if source == "blynk":
        add_log("ONLINE OPEN (Blynk)")
    cancel_actions_of(close_gate)
    schedule_action(close_gate, GATE_OPEN_TIME)

def close_gate():
    global gate_is_open
    gate_is_open = False
    set_servo(SERVO_CLOSED)
    blynk_write_safe(6, 0)

# ─────────────────────────────────────────────────────────────
# BLYNK HANDLERS
# ─────────────────────────────────────────────────────────────

if blynk:

    @blynk.on("V1")
    def relay1_handler(value):
        RELAY_1.value(int(value[0]))

    @blynk.on("V2")
    def relay2_handler(value):
        RELAY_2.value(int(value[0]))

    @blynk.on("V3")
    def relay3_handler(value):
        RELAY_3.value(int(value[0]))

    @blynk.on("V4")
    def relay4_handler(value):
        RELAY_4.value(int(value[0]))

    @blynk.on("V5")
    def terminal_handler(value):
        global people_inside, users_inside
        cmd = value[0].strip().lower()

        if cmd == "status":
            ids = ", ".join(users_inside) if users_inside else "None"
            blynk_write_safe(5, f"People inside : {people_inside}\n")
            blynk_write_safe(5, f"IDs inside    : {ids}\n")
            blynk_write_safe(5, f"Gate open     : {gate_is_open}\n")
            blynk_write_safe(5, f"Uptime        : {get_uptime()}\n")

        elif cmd == "logs":
            blynk_write_safe(5, "──── ACCESS LOG (last 10) ────\n")
            for log in (access_logs[-10:] if access_logs else ["No logs yet"]):
                blynk_write_safe(5, log + "\n")
            blynk_write_safe(5, "──────────────────────────────\n")

        elif cmd == "users":
            blynk_write_safe(5, f"Registered : {', '.join(USERS.keys())}\n")
            blynk_write_safe(5, f"Inside now : {', '.join(users_inside) if users_inside else 'None'}\n")

        elif cmd == "reset":
            people_inside = 0
            users_inside.clear()
            save_state()
            add_log("COUNT RESET via terminal")
            blynk_write_safe(5, "Count / inside-list reset.\n")

        elif cmd in ("help", "?"):
            blynk_write_safe(5, "Commands: status | logs | users | reset | clear | help\n")

        elif cmd == "clear":
            blynk_write_safe(5, "\n" * 10)

        else:
            blynk_write_safe(5, f"Unknown: '{cmd}' — type 'help'\n")

    @blynk.on("V6")
    def gate_handler(value):
        if int(value[0]):
            open_gate(source="blynk")
        else:
            close_gate()

# ─────────────────────────────────────────────────────────────
# PIR — no logging, only Blynk LED on V7
# ─────────────────────────────────────────────────────────────

def check_pir():
    global last_pir, last_pir_time
    if gate_is_open:
        return
    state = pir.value()
    now   = time.ticks_ms()
    if state != last_pir and time.ticks_diff(now, last_pir_time) > 500:
        last_pir      = state
        last_pir_time = now
        blynk_write_safe(7, 255 if state else 0)

# ─────────────────────────────────────────────────────────────
# KEYPAD HANDLER
# ─────────────────────────────────────────────────────────────

def handle_keypad():
    global screen, input_id, input_pw, people_inside, users_inside, input_timeout

    key = scan_keypad()
    if key is None:
        return
    blink_led(1, 50)
    print(f"Key={key} Screen={screen}")

    # # = Home always
    if key == '#':
        go_home(); return

    # * = backspace current field
    if key == '*':
        if   screen == "ENTRY_ID": input_id = input_id[:-1]; show_entry_id()
        elif screen == "ENTRY_PW": input_pw = input_pw[:-1]; show_entry_pw()
        elif screen == "EXIT_ID":  input_id = input_id[:-1]; show_exit_id()
        return

    # A = Entry mode, B = Exit mode (from any screen)
    if key == 'A':
        input_id = ""; input_pw = ""
        input_timeout = time.ticks_add(time.ticks_ms(), INPUT_TIMEOUT)
        show_entry_id(); return

    if key == 'B':
        input_id = ""; input_pw = ""
        input_timeout = time.ticks_add(time.ticks_ms(), INPUT_TIMEOUT)
        show_exit_id(); return

    # HOME: C/D do nothing
    if screen == "HOME":
        return

    # ── ENTRY - USER ID ─────────────────────────────────
    if screen == "ENTRY_ID":
        if key == 'C':
            if not input_id:
                lcd_show("ID is empty!", "Enter user ID")
                schedule_action(show_entry_id, 1500)
            elif input_id not in USERS:
                add_log(f"ENTRY DENIED — Unknown ID:{input_id}")
                lcd_show("Unknown User!", "Not registered")
                input_id = ""
                schedule_action(show_entry_id, 2000)
            elif input_id in users_inside:
                add_log(f"ENTRY DENIED — ID:{input_id} already inside")
                lcd_show("Already Inside!", f"ID:{input_id} in use")
                input_id = ""
                schedule_action(show_entry_id, 2000)
            else:
                input_pw = ""
                input_timeout = time.ticks_add(time.ticks_ms(), INPUT_TIMEOUT)
                show_entry_pw()

        elif key in '0123456789':
            if len(input_id) < 8: input_id += key; show_entry_id()
        return

    # ── ENTRY - PASSWORD ────────────────────────────────
    if screen == "ENTRY_PW":
        if key == 'C':
            if USERS.get(input_id) == input_pw:
                people_inside += 1
                users_inside.add(input_id)
                save_state()
                add_log(f"ENTRY — ID:{input_id} | Inside:{people_inside}")
                lcd_show("ACCESS GRANTED", f"Welcome {input_id}!")
                blink_led(3, 100)
                try: bt.write(f"ENTRY:{input_id}\n".encode())
                except: pass
                input_timeout = 0
                open_gate(source="local")
                schedule_action(go_home, GATE_OPEN_TIME + 500)
            else:
                add_log(f"ENTRY DENIED — Wrong password ID:{input_id}")
                lcd_show("Wrong Password!", "Access Denied")
                blink_led(5, 50)
                try: bt.write(b"LOGIN:FAIL\n")
                except: pass
                input_timeout = 0
                schedule_action(go_home, 2000)

        elif key == 'D':
            input_pw = ""
            input_timeout = time.ticks_add(time.ticks_ms(), INPUT_TIMEOUT)
            show_entry_id()

        elif key in '0123456789':
            if len(input_pw) < 8: input_pw += key; show_entry_pw()
        return

    # ── EXIT - USER ID ──────────────────────────────────
    if screen == "EXIT_ID":
        if key == 'C':
            if not input_id:
                lcd_show("ID is empty!", "Enter user ID")
                schedule_action(show_exit_id, 1500)
            elif input_id not in USERS:
                add_log(f"EXIT DENIED — Unknown ID:{input_id}")
                lcd_show("Unknown User!", "Not registered")
                input_id = ""
                schedule_action(show_exit_id, 2000)
            elif input_id not in users_inside:
                add_log(f"EXIT DENIED — ID:{input_id} not inside")
                lcd_show("Not Inside!", "You didn't enter")
                blink_led(3, 80)
                try: bt.write(f"EXIT_DENIED:{input_id}\n".encode())
                except: pass
                input_id = ""
                schedule_action(show_exit_id, 2500)
            else:
                users_inside.discard(input_id)
                people_inside = len(users_inside)
                save_state()
                add_log(f"EXIT — ID:{input_id} | Inside:{people_inside}")
                lcd_show("Exit Recorded", f"Inside:{people_inside}")
                try: bt.write(f"EXIT:{input_id}\n".encode())
                except: pass
                input_timeout = 0
                open_gate(source="local")
                schedule_action(go_home, GATE_OPEN_TIME + 500)

        elif key == 'D':
            go_home()

        elif key in '0123456789':
            if len(input_id) < 8: input_id += key; show_exit_id()
        return

# ─────────────────────────────────────────────────────────────
# TIMEOUT CHECKER
# ─────────────────────────────────────────────────────────────

def check_timeout():
    global input_timeout
    if input_timeout > 0 and screen in ("ENTRY_ID", "ENTRY_PW", "EXIT_ID"):
        if time.ticks_diff(time.ticks_ms(), input_timeout) >= 0:
            input_timeout = 0
            lcd_show("Timed out!", "Going home...")
            cancel_actions_of(show_entry_id)
            cancel_actions_of(show_entry_pw)
            cancel_actions_of(show_exit_id)
            schedule_action(go_home, 1500)

# ─────────────────────────────────────────────────────────────
# WIFI CHECK (moved to function — fixes global SyntaxError)
# ─────────────────────────────────────────────────────────────

def check_wifi():
    global last_wifi_check, wlan
    now_ms = time.ticks_ms()
    if time.ticks_diff(now_ms, last_wifi_check) >= WIFI_CHECK_INTERVAL:
        last_wifi_check = now_ms
        if wlan and not wlan.isconnected():
            print("WiFi lost — reconnecting...")
            wlan = connect_wifi()
            if wlan and wlan.isconnected():
                add_log("WiFi reconnected")

# ─────────────────────────────────────────────────────────────
# INIT
# ─────────────────────────────────────────────────────────────

print("=" * 50)
print("SMART ACCESS CONTROL SYSTEM v3 - FINAL")
print("=" * 50)

lcd_init()
lcd_show("System Ready", f"Inside:{people_inside}")
time.sleep(2)
go_home()
set_servo(SERVO_CLOSED)

for i in range(4):
    blynk_write_safe(i + 1, 0)

add_log(f"System started | Inside:{people_inside}")
print("Ready!")
print("=" * 50)

# ─────────────────────────────────────────────────────────────
# MAIN LOOP
# ─────────────────────────────────────────────────────────────

while True:
    try:
        if blynk: blynk.run()
        check_pir()
        handle_keypad()
        check_timeout()
        check_delayed_actions()
        check_wifi()            # ← function call, no global needed here
        time.sleep_ms(10)

    except KeyboardInterrupt:
        print("\nShutdown...")
        lcd_show("Shutting Down", "Goodbye!")
        set_servo(SERVO_CLOSED)
        for r in relays: r.value(0)
        led.value(0)
        break

    except Exception as e:
        print(f"ERROR: {e}")
        import sys
        sys.print_exception(e)
        time.sleep(1)

