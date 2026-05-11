# RULES_CHECKLIST.md

Project: Technocorner 2026 Sumobot RC  
Purpose: quick legality + match-day checklist.  
Mode: RC only. No autonomous goblin brain.

---

## 1. Legal Robot Gate

Must pass all.

- [ ] Robot is Remote-Controlled only.
- [ ] Robot is not bought ready-made/pre-built.
- [ ] Footprint <= 10 cm x 10 cm in starting position.
- [ ] Mass <= 500 g including battery.
- [ ] Height OK. Unlimited, but stable.
- [ ] Front label visible: robot name + all member names.
- [ ] Robot/team name has no offensive/SARA/porn/harassment/radical/provocation content.

---

## 2. Banned Mechanism Gate

None allowed.

- [ ] No suction cup / diaphragm / sticky wheel.
- [ ] No vacuum pump.
- [ ] No magnet/downforce device.
- [ ] No signal jammer.
- [ ] No strobe light.
- [ ] No laser.
- [ ] No projectile.
- [ ] No saw.
- [ ] No flammable device.
- [ ] No arena-damaging part.
- [ ] No throwing/flipping mechanism.
- [ ] No intentional split/detachable robot parts.

---

## 3. Control + Firmware Gate

RC must stay RC.

- [ ] Robot only moves from handler input.
- [ ] No autonomous line sensor / opponent tracking / auto-attack.
- [ ] Remote has arm/disarm.
- [ ] Signal loss -> robot stops.
- [ ] `stopRobot()` tested.
- [ ] Motor direction tested: forward, backward, left, right.
- [ ] Remote can be turned off when not competing.

---

## 4. Electrical + Mechanical Gate

No smoke. No loose goblin parts.

- [ ] Battery voltage matches motor driver + MCU regulator.
- [ ] Motor stall current safe for driver.
- [ ] Power switch accessible.
- [ ] Battery strapped firmly.
- [ ] Wires clear from wheels.
- [ ] No exposed positive terminal.
- [ ] Wheels tight.
- [ ] Wedge/plow tight.
- [ ] No sharp unsafe edge.

Notes:

```txt
MCU:
Motor driver:
Motor:
Battery:
Regulator/BEC:
Wheel:
Mass:
Width x Length:
```

---

## 5. Arena + Strategy Gate

Arena: black HPL hardwood circle, diameter 77 cm, white edge line 2.5 cm.

- [ ] Grip tested on smooth surface.
- [ ] Robot pushes straight.
- [ ] Robot can reverse when stuck.
- [ ] Robot can turn in place.
- [ ] Driver can see boundary.
- [ ] Robot fits center start position.

---

## 6. Admin Gate

Do not lose before robot fights.

- [ ] Registered on official Technocorner website.
- [ ] Payment done through official website.
- [ ] Team joined official WhatsApp group.
- [ ] Twibbon uploaded by all members.
- [ ] Instagram accounts public until registration ends.
- [ ] Technical Meeting attended or notes received.
- [ ] Identity card ready.
- [ ] Semi-formal photo ready.
- [ ] Official ID card worn at venue.

---

## 7. Match Gate

Before every match.

- [ ] Handler + assistant are registered members.
- [ ] Only handler + assistant approach arena.
- [ ] Battery charged before match.
- [ ] Remote charged.
- [ ] 1-minute prep routine practiced.
- [ ] No battery/component replacement during match.
- [ ] Other team remotes turned off when not competing.
- [ ] Team ready before third call. Third miss = DQ risk.

---

## 8. Scoring Memory

- Push opponent out -> +1.
- Opponent leaves arena and touches floor last -> +1.
- Robot cannot move for 10 s -> opponent +1.
- Group stage: 3 rounds, 4 min, no timeout.
- Knockout: 3 rounds, 5 min, 1 timeout per match, 45 s.
- Referee/judge decision final.

---

## Final Pass

```txt
Legal gate: PASS / FAIL
Control gate: PASS / FAIL
Electrical gate: PASS / FAIL
Mechanical gate: PASS / FAIL
Admin gate: PASS / FAIL
Driver gate: PASS / FAIL

Ready for inspection: YES / NO
Ready for match: YES / NO

Checked by:
Date:
```
