#include "../components/feiya_desk/preset_control.h"
#include "../components/feiya_desk/protocol.h"
#include <cassert>
#include <iostream>

int main() {
  feiya_protocol::PresetControl c;
  assert(c.key(0) == 0);
  for (unsigned i = 0; i <= 255; ++i) {
    if (i < 1 || i > 4) {
      assert(!c.start(static_cast<uint8_t>(i), 0));
      assert(!c.save(static_cast<uint8_t>(i), 0));
    }
  }
  assert(c.start(1, 100));
  assert((feiya_protocol::command(c.key(100)) ==
          std::array<uint8_t, 5>{0xA5, 0, 0x02, 0xFD, 0xFF}));
  assert(!c.start(1, 299));  // Repeated requests cannot extend a press.
  assert(!c.start(2, 299));  // Nor overlap two different buttons.
  assert(c.key(299) == 0x02);
  assert(c.key(300) == 0);
  assert((feiya_protocol::command(c.key(301)) ==
          std::array<uint8_t, 5>{0xA5, 0, 0, 0xFF, 0xFF}));
  assert(c.start(2, 400));
  assert((feiya_protocol::command(c.key(400)) ==
          std::array<uint8_t, 5>{0xA5, 0, 0x04, 0xFB, 0xFF}));
  c.release();
  assert(c.key(401) == 0);
  assert(c.start(2, UINT32_MAX - 100));
  assert(c.key(98) == 0x04);
  assert(c.key(99) == 0);
  assert(c.key(100000) == 0);  // No automatic repetition or restart.

  // Exact four preset frames from the user's intercepted-command photograph.
  const std::array<std::array<uint8_t, 5>, 4> slots{{
      {0xA5, 0, 0x02, 0xFD, 0xFF}, {0xA5, 0, 0x04, 0xFB, 0xFF},
      {0xA5, 0, 0x08, 0xF7, 0xFF}, {0xA5, 0, 0x10, 0xEF, 0xFF}}};
  for (unsigned slot = 1; slot <= 4; ++slot) {
    assert(c.start(slot, 0));
    assert(feiya_protocol::command(c.key(0)) == slots[slot - 1]);
    assert(c.key(200) == 0);
    assert(c.save(slot, 1000));
    assert((feiya_protocol::command(c.key(1000)) ==
            std::array<uint8_t, 5>{0xA5, 0, 0x01, 0xFE, 0xFF}));
    assert(c.key(1999) == 0x01);
    assert(!c.start(1, 1999) && !c.save(2, 1999));
    assert(c.key(2000) == 0);
    assert(c.key(2299) == 0);
    assert(!c.start(1, 2299) && !c.save(2, 2299));  // Gap is still busy.
    assert(feiya_protocol::command(c.key(2300)) == slots[slot - 1]);
    assert(feiya_protocol::command(c.key(2499)) == slots[slot - 1]);
    assert(c.key(2500) == 0);
  }
  assert(!c.save(0, 0) && !c.save(5, 0));
  // Cancelled or delayed saves must never emit an orphaned preset recall.
  assert(c.save(1, 0));
  assert(c.key(1000) == 0);
  c.release();
  assert(c.key(1300) == 0);
  assert(c.save(1, 0));
  assert(c.key(1600) == 0);
  assert(c.key(1900) == 0);
  assert(c.save(1, 0));
  assert(c.key(1000) == 0);
  assert(c.key(1900) == 0);
  assert(c.key(2000) == 0);
  assert(c.save(2, UINT32_MAX - 100));
  assert(c.key(898) == 0x01);
  assert(c.key(899) == 0);
  assert(c.key(1198) == 0);
  assert(c.key(1199) == 0x04);
  assert(c.key(1399) == 0);

  assert(!c.jog(0x60, 0) && !c.jog(0x01, 0));
  assert(c.jog(0x20, 100));
  assert(c.key(599) == 0x20);
  assert(!c.jog(0x40, 599) && !c.save(1, 599));
  assert(!c.jog(0x20, 599, 5000));  // Rejected long jog cannot lengthen this one.
  assert(c.key(600) == 0);
  assert(c.jog(0x40, UINT32_MAX - 100));
  assert(c.key(398) == 0x40);
  assert(c.key(399) == 0);
  for (uint32_t duration : {0U, 499U, 501U, 4999U, 5001U, UINT32_MAX})
    assert(!c.jog(0x20, 0, duration));
  for (uint8_t direction : {0x20, 0x40}) {
    assert(c.jog(direction, 1000, 5000));
    assert(c.is_jog());
    assert(c.key(1499) == direction);
    assert(!c.jog(direction, 1499));  // Cannot shorten a five-second move either.
    assert(!c.start(3, 2000) && !c.save(4, 2000));
    assert(c.key(5999) == direction);
    assert(!c.jog(direction, 5999, 5000));  // No extension.
    assert(c.key(6000) == 0 && !c.is_jog());
  }
  assert(c.jog(0x20, 1000, 5000));
  assert(c.cancel_manual());
  assert(c.key(1001) == 0);
  assert(!c.cancel_manual());
  assert(c.jog(0x40, UINT32_MAX - 100, 5000));
  assert(c.key(4898) == 0x40);
  assert(c.key(4899) == 0);
  assert(c.save(4, 0));
  assert(!c.cancel_manual());  // Manual cancel must not corrupt a save sequence.
  assert(c.key(999) == 0x01);
  c.release();  // Network/data loss cancels any operation.
  assert(c.key(1000) == 0 && c.key(1300) == 0);
  std::cout << "FEIYA preset command tests passed\n";
}
