# agents.md

Scope: whole repo.

Read first:
- `@context.md` -> Technocorner 2026 Sumobot RC rules, match system, robot constraints.
- `@caveman.md` -> response style, compression rules.

This repo builds RC sumobot for Technocorner 2026. Goal: pass inspection, drive hard, push opponent out, stay legal, ship fast.

---

## 0. Prime Directive

Win condition:
- Legal robot.
- Reliable RC control.
- Strong drivetrain.
- Good traction.
- Simple wedge.
- Fast driver.
- No banned mechanisms.

Default decision rule:

`legal -> reliable -> strong -> light -> simple -> pretty`

Never optimize pretty before robot moves.

---

## 1. Caveman Ultra Mode

Use this style unless user asks otherwise.

Rules:
- Zero filler.
- No pleasantries.
- Drop articles when safe.
- Use fragments.
- Use abbreviations: cfg, fn, dir, lib, hw, fw, mech, elec, tx, rx, bot.
- Use arrows: `->`, `=>`.
- Keep code exact. Never compress syntax.
- Keep commands exact.
- Keep math exact.
- Use standard prose only for safety-critical warnings, destructive commands, irreversible hardware risk.

Output shape:

```md
Goal -> ...
Do -> ...
Files -> ...
Cmd -> ...
Risk -> ...
Next -> ...
```

No motivational fluff. No corporate prose. Smart caveman only.

---

## 2. Competition Context Canon

Event:
- Technocorner 2026 Sumobot RC.
- Theme: The Eternal Flight of Nusantara: From Heritage to Horizon.
- Location: Faculty of Engineering UGM.
- Competition day: 25 Jun 2026.
- Awarding: 27 Jun 2026.
- Technical Meeting: 3 Jun 2026, online.
- Registration: 17 Mar 2026 to 25 May 2026, online.

Robot mission:
- Push opponent robot out designated arena.
- Robot must be Remote-Controlled.
- No autonomous behavior.

Team:
- 2 to 3 members.
- Optional 1 supervisor.
- Each person may join 1 team only.
- Each robot may register 1 team only.

Robot hard limits:
- Max footprint: 10 cm x 10 cm in starting position.
- Height: unlimited.
- Max mass: 500 g including battery, excluding remote.
- Front label required: team member names + robot name, visible to referee.

Arena:
- Hardwood.
- Black HPL finish.
- Circle diameter: 77 cm.
- Height: 2.5 cm.
- White boundary line at edge: 2.5 cm width.
- Brown start marks: 10 cm x 1 cm.
- Distance between start marks: 10 cm.

Banned:
- Arena-damaging devices.
- Throw-flip devices.
- Signal jammer.
- Strobe light.
- Laser.
- Suction cup.
- Diaphragm adhesion.
- Sticky wheel.
- Projectile weapon.
- Saw weapon.
- Flammable device.
- Robot splitting into parts.
- Vacuum pump.
- Magnet downforce.

Inspection checks:
- Dimension.
- Weight.
- Suction.
- Sharp parts.
- Admin eligibility.

Match basics:
- Group stage first.
- 4 teams per group.
- Top 2 teams by points proceed knockout.
- Knockout pairing randomized.
- Match operated by 1 handler + 1 assistant.
- Only registered handler + assistant approach arena.
- Before match: 1 min prep time in sterile area.
- Referee, committee, judge decision final.

Group stage:
- 3 rounds.
- 4 min total match duration.
- Best of 3.
- No timeout.
- Tiebreak: win time, then survive time.

Knockout:
- 3 rounds.
- 5 min total match duration.
- Best of 3.
- 1 timeout per match, 45 s.
- Timeout only after round ends, before next round starts.
- No battery/component replacement during timeout.
- If neither team scores -> both do not proceed.

Scoring:
- +1: push opponent out arena.
- +1: opponent leaves arena and touches floor last.
- +1 to opponent: own robot cannot move for 10 s.

DQ risks:
- Rule violation.
- Disturbance.
- Failed requirements.
- Cheating.
- Plagiarism.
- Sabotage.
- No-show after call logic.

---

## 3. Repo Shape

Preferred structure:

```txt
sumobot-rc/
├── AGENTS.md
├── CONTEXT.md
├── caveman.md
├── README.md
├── docs/
│   ├── bom.md
│   ├── wiring.md
│   ├── rules-notes.md
│   ├── inspection-checklist.md
│   ├── match-checklist.md
│   ├── test-log.md
│   └── driver-drills.md
├── firmware/
│   ├── receiver_robot/
│   │   ├── platformio.ini
│   │   └── src/
│   │       ├── main.cpp
│   │       ├── pins.h
│   │       ├── config.h
│   │       ├── protocol.h
│   │       ├── motor_driver.h
│   │       ├── motor_driver.cpp
│   │       ├── failsafe.h
│   │       └── failsafe.cpp
│   └── transmitter_remote/
│       ├── platformio.ini
│       └── src/
│           ├── main.cpp
│           ├── pins.h
│           ├── config.h
│           └── protocol.h
├── hardware/
│   ├── cad/
│   ├── chassis/
│   ├── electronics/
│   ├── wiring/
│   └── photos/
└── tests/
    ├── bench/
    ├── drive/
    ├── comms/
    └── weigh-in/
```

Keep docs short. One fact per line. Tables okay.

---

## 4. Firmware Default Stack

Default MCU:
- ESP32.

Default framework:
- PlatformIO + Arduino framework.

Default comms:
- ESP-NOW for ESP32 tx/rx.
- Alternative: hobby RC receiver PWM/SBUS if hardware already exists.

Default drive:
- 2 brushed DC motors.
- Differential/tank drive.
- Motor driver: TB6612FNG, DRV8833, BTS7960, or equivalent sized to motor stall current.

Default firmware features:
- Joystick throttle.
- Joystick steering.
- Differential mix.
- Arm switch.
- Failsafe stop on lost packet.
- Motor direction calibration.
- Speed limit cfg.
- Deadzone cfg.
- No autonomous logic.

Common libs:

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
```

Avoid extra libs unless needed. Less deps -> less failure.

---

## 5. Build Commands

Receiver:

```sh
cd firmware/receiver_robot
pio run
pio run -t upload
pio device monitor
```

Transmitter:

```sh
cd firmware/transmitter_remote
pio run
pio run -t upload
pio device monitor
```

Clean:

```sh
pio run -t clean
```

If Arduino IDE project:
- Keep `.ino` simple.
- Still split constants into `config.h`, `pins.h`, `protocol.h` if possible.

---

## 6. Code Style

C++ rules:
- Use `constexpr` or `const` for pins/cfg.
- Use `int16_t`, `uint8_t`, `uint32_t` for packet structs.
- Keep packet struct same on tx/rx.
- No dynamic allocation.
- No `String` in firmware hot path.
- No blocking delay in receiver loop except fatal init loop.
- Keep loop deterministic.
- Prefer small pure fn.
- Clamp motor output.
- Deadzone joystick input.
- Failsafe stop when packet stale.

Naming:
- Pins: `PIN_L_PWM`, `PIN_R_IN1`.
- Config: `PWM_FREQ`, `FAILSAFE_MS`, `DEADZONE`.
- Functions: `setupMotors()`, `setMotor()`, `stopRobot()`.
- Packet: `ControlPacket`.

Packet rules:
- Never reorder packet fields without updating both tx/rx.
- Add version byte if protocol grows.
- Keep packet tiny.

Motor rules:
- Forward must match wedge-forward.
- Add invert flags in cfg, not rewiring every test.
- Brake/coast mode must be explicit.

---

## 7. Hardware Defaults

Mechanical priority:
1. Fits 10 cm x 10 cm.
2. Under 500 g with battery.
3. Low center of mass.
4. Strong front wedge.
5. Good traction, legal wheels.
6. Serviceable battery access.
7. Protected electronics.

Electrical priority:
1. Motor stall current safe.
2. Battery can supply peak current.
3. Common ground between MCU and driver.
4. Brownout prevention.
5. Power switch reachable.
6. Wires strain-relieved.
7. No exposed short risk.

Illegal design reject instantly:
- Magnet.
- Vacuum.
- Suction.
- Sticky wheels.
- Saw.
- Projectile.
- Laser/strobe.
- Jammer.
- Split body.

---

## 8. Agent Roles

Use these mental agents when planning work.

### Rules Agent

Job:
- Check proposed design against competition rules.
- Block illegal mechanism fast.
- Maintain `docs/rules-notes.md`.
- Maintain `docs/inspection-checklist.md`.

Output:

```md
Status -> legal / risky / illegal
Rule hit -> ...
Fix -> ...
```

### Firmware Agent

Job:
- Implement tx/rx control.
- Keep failsafe reliable.
- Keep code small.
- Maintain `firmware/*`.

Must check:
- Build passes.
- Packet struct same tx/rx.
- Failsafe stops motors.
- Arm switch works.
- Motor direction correct.
- No autonomous behavior.

Output:

```md
Files -> ...
Behavior -> ...
Test -> ...
Risk -> ...
```

### Electrical Agent

Job:
- Select battery, motor driver, wiring, fuse/switch.
- Prevent brownout and burnt driver.
- Maintain `docs/wiring.md`, `docs/bom.md`.

Must check:
- Motor stall current vs driver rating.
- Battery C rating vs peak current.
- ESP32 powered safely.
- Ground shared.
- Wire gauge adequate.

Output:

```md
Supply -> ...
Driver -> ...
Current risk -> ...
Fix -> ...
```

### Mechanical Agent

Job:
- Chassis, wedge, wheel placement, weight balance.
- Maintain CAD and `hardware/` docs.

Must check:
- 10 cm x 10 cm starting footprint.
- Under 500 g.
- No sharp illegal parts.
- No arena damage.
- Wedge touches low without scraping too hard.

Output:

```md
Mass -> ...
Footprint -> ...
Wedge -> ...
Risk -> ...
```

### Driver Coach Agent

Job:
- Convert robot behavior into driver drills.
- Maintain `docs/driver-drills.md`.
- Track win time and survive time.

Must check:
- Start reaction.
- Straight push.
- Edge recovery.
- Side escape.
- Lock handling.
- Battery sag behavior.

Output:

```md
Drill -> ...
Metric -> ...
Target -> ...
Next run -> ...
```

### Match Strategy Agent

Job:
- Plan group and knockout strategy.
- Minimize DQ risk.
- Exploit scoring rules legally.

Must check:
- Group: no timeout.
- Knockout: timeout allowed once, 45 s.
- Immobile 10 s gives opponent point.
- Fast win time matters for tiebreak.
- Survive time matters for tiebreak.

Output:

```md
Stage -> group / knockout
Goal -> ...
Risk -> ...
Driver call -> ...
```

### Documentation Agent

Job:
- Keep docs current.
- Compress facts.
- Make checklists usable at venue.

Must check:
- One source of truth.
- No stale pinout.
- BOM matches actual bot.
- Photos updated after wiring changes.

Output:

```md
Updated -> ...
Stale -> ...
Need photo -> yes/no
```

---

## 9. Development Workflow

Every change:

```txt
Understand rule -> edit smallest files -> build/test -> update doc -> log result
```

Before changing firmware:
- Read relevant `config.h`, `pins.h`, `protocol.h`.
- Check tx/rx packet compatibility.
- Avoid changing public packet fields unless necessary.

Before changing hardware:
- Check mass budget.
- Check footprint.
- Check banned mechanisms.
- Update wiring/BOM/docs.

Before match day:
- Freeze firmware.
- Freeze wiring.
- Bring spare battery, screwdriver, tape, zip ties, charger.
- Do not make risky last-minute feature changes.

---

## 10. Test Gates

### Bench Gate

Pass when:
- Bot powers on.
- Remote arms/disarms.
- Wheels spin correct direction.
- Lost signal stops motors.
- Battery does not brown out ESP32.
- Motor driver not overheating after short push test.

### Floor Gate

Pass when:
- Drives straight.
- Turns both directions.
- Stops on disarm.
- Recovers from edge command.
- No loose wire after impact.

### Arena-Sim Gate

Pass when:
- Fits 10 cm x 10 cm box.
- Mass under 500 g.
- Pushes test object.
- Survives side contact.
- Driver can stop before edge.

### Venue Gate

Pass when:
- ID card ready.
- Robot label visible.
- Battery charged.
- Remote off when not competing.
- Team knows call order.
- Handler + assistant assigned.

---

## 11. Definition of Done

Firmware change done when:
- Compiles.
- Uploaded to correct board.
- Bench tested.
- Failsafe tested.
- Docs updated if pins/protocol/cfg changed.

Mechanical change done when:
- Fits 10 cm x 10 cm.
- Mass measured.
- No banned features.
- Screws tight.
- Sharp edges handled.
- Photo saved.

Electrical change done when:
- Wiring diagram updated.
- Polarity checked.
- Common ground checked.
- Current risk checked.
- Heat checked.

Strategy change done when:
- Driver can execute.
- Rule legal.
- Measured in test log.

---

## 12. Fast Task Templates

### Firmware Task

```md
Task -> ...
Files -> ...
Expected behavior -> ...
Build cmd -> ...
Bench test -> ...
Rollback -> ...
```

### Hardware Task

```md
Task -> ...
Part -> ...
Mass delta -> ...
Footprint impact -> ...
Rule risk -> ...
Test -> ...
```

### Test Log Entry

```md
Date -> YYYY-MM-DD
Bot cfg -> ...
Battery -> ...
Surface -> ...
Test -> ...
Result -> ...
Fail -> ...
Next -> ...
```

### Match Checklist Entry

```md
Before call -> battery, remote, label, screws, wheels
At sterile area -> arm off, remote ready, 1 min prep
During round -> push, avoid edge, recover fast
After round -> inspect wheels, heat, screws, battery sag
```

---

## 13. AI Editing Rules

When agent edits code:
- Prefer minimal patch.
- Do not rewrite whole project unless asked.
- Preserve pin config unless task needs change.
- Preserve packet ABI unless task needs change.
- Add test steps after change.
- Mention hardware assumptions.

When agent answers question:
- Use Caveman Ultra.
- Cite `CONTEXT.md` facts when discussing rules.
- Do not invent competition rules.
- If rule unknown, say unknown and ask team to verify in TM or official guidebook.

When agent reviews design:
- Check legality first.
- Then reliability.
- Then performance.
- Then maintainability.

When agent sees banned feature:
- Mark illegal.
- Explain exact risk.
- Suggest legal alternative.

---

## 14. Common Good Defaults

Good design defaults:
- Two-wheel differential drive.
- Passive wedge.
- Low CG.
- Short wires.
- Locking connectors if possible.
- Rubber wheels, legal non-sticky traction.
- Exposed power switch.
- Spare battery.
- Simple remote arm switch.

Bad design defaults:
- Too many sensors.
- Autonomous line/opponent logic.
- Fragile 3D print-only wedge.
- Tiny motor driver below stall current.
- Tall battery stack.
- Loose Dupont wires near wheels.
- Last-minute firmware rewrite.

---

## 15. Critical Warnings

Do not test full-power motors with wheels touching fingers, cables, table edge, or loose parts. Lift bot safely or remove wheels for first spin test.

Do not short LiPo/Li-ion battery. Check polarity before plugging. Use proper charger. Stop using swollen/hot/damaged battery.

Do not upload untested firmware right before match unless current firmware is broken. Last-minute cleverness kills robots.

---

## 16. Team Speed Rules

Daily standup format:

```md
Done -> ...
Blocked -> ...
Next -> ...
Risk -> ...
Need -> ...
```

Priority board:

```txt
P0 -> legal / inspection / motion / safety
P1 -> comms / traction / battery / wedge
P2 -> driver drills / docs / spares
P3 -> cosmetics / telemetry / nice-to-have
```

No debate longer than 10 min without test.

Opinion loses to measured result.

Rulebook beats vibes.

