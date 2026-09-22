#include "../components/feiya_desk/protocol.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using feiya_protocol::DisplayFrame;
using feiya_protocol::DisplayKind;
using feiya_protocol::DisplayParser;

static std::vector<DisplayFrame> feed(DisplayParser &parser, const std::vector<uint8_t> &bytes,
                                     uint32_t start = 0) {
  std::vector<DisplayFrame> results;
  for (auto byte : bytes) {
    DisplayFrame frame;
    if (parser.push(byte, start++, frame)) results.push_back(frame);
  }
  return results;
}

int main() {
  DisplayParser parser;
  // Exact captured frames, matched to the user's displayed heights.
  auto frames = feed(parser, {0x5A, 0x07, 0x87, 0x3F, 0xCD,
                              0x5A, 0x07, 0xED, 0x7F, 0x73});
  assert(frames.size() == 2);
  assert(frames[0].kind == DisplayKind::HEIGHT && std::fabs(frames[0].height_cm - 77.0f) < 0.001f);
  assert(frames[1].kind == DisplayKind::HEIGHT && std::fabs(frames[1].height_cm - 75.8f) < 0.001f);
  frames = feed(parser, {0x5A, 0, 0, 0, 0});
  assert(frames.size() == 1 && frames[0].kind == DisplayKind::BLANK);

  // Wrong header/checksum must not create height readings; recover on next frame.
  frames = feed(parser, {0x54, 0x07, 0x87, 0x3F, 0xCD,
                         0x5A, 0x07, 0x87, 0x3F, 0xCC,
                         0x5A, 0x07, 0xED, 0x7F, 0x73});
  assert(frames.size() == 1 && std::fabs(frames[0].height_cm - 75.8f) < 0.001f);
  // Truncated frame followed by a new header, including a header as checksum.
  frames = feed(parser, {0x5A, 0x07, 0x87, 0x3F, 0x5A, 0x07, 0xED, 0x7F, 0x73});
  assert(frames.size() == 1 && frames[0].kind == DisplayKind::HEIGHT);
  assert(feed(parser, {0x5A, 0x07}, 1000).empty());
  assert(feed(parser, {0x87, 0x3F, 0xCD}, 1200).empty());
  assert(feed(parser, {0x5A, 0x07, 0x87, 0x3F, 0xCD}, 1203).size() == 1);

  // Invalid segments, interior blanks, and multiple decimal points aren't heights.
  for (auto raw : std::vector<std::array<uint8_t, 3>>{{0x79, 0x3F, 0x06},
                                                    {0x07, 0x00, 0x7F},
                                                    {0x87, 0x87, 0x3F}}) {
    frames = feed(parser, {0x5A, raw[0], raw[1], raw[2],
                           static_cast<uint8_t>(raw[0] + raw[1] + raw[2])});
    assert(frames.size() == 1 && frames[0].kind == DisplayKind::OTHER);
  }
  // Numeric formats beyond captured heights: supported, still need hardware checks.
  frames = feed(parser, {0x5A, 0x06, 0x3F, 0x3F, 0x84});
  assert(frames.size() == 1 && frames[0].height_cm == 100.0f);
  // Wraparound of the millisecond clock must not discard a complete valid frame.
  frames = feed(parser, {0x5A, 0x07, 0x87, 0x3F, 0xCD}, UINT32_MAX - 2);
  assert(frames.size() == 1 && frames[0].height_cm == 77.0f);
  feiya_protocol::ManualControl control;
  assert(control.key(0) == 0);
  assert(!control.start(0x60, 0));  // Never send simultaneous up and down.
  assert(!control.start(0x01, 0));  // No unverified M/reset commands.
  assert(control.start(0x20, 100));
  assert(control.key(599) == 0x20);
  assert(!control.start(0x20, 599));  // Repeated clicks cannot extend a jog.
  assert(!control.start(0x40, 599));
  assert(control.key(600) == 0);
  assert(control.start(0x40, 601));
  control.cancel();
  assert(control.key(602) == 0);
  assert(control.start(0x20, UINT32_MAX - 100));
  assert(control.key(398) == 0x20);
  assert(control.key(399) == 0);
  assert((feiya_protocol::command(0x20) == std::array<uint8_t, 5>{0xA5, 0, 0x20, 0xDF, 0xFF}));
  assert((feiya_protocol::command(0) == std::array<uint8_t, 5>{0xA5, 0, 0, 0xFF, 0xFF}));
  std::cout << "FEIYA display protocol tests passed\n";
}
