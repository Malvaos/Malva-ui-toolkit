# Token reference

_Generated from `ui/tokens.slint`. Every component reads only from the `Theme` global._

| Token | Type | Value |
|---|---|---|
| `Theme.dark` | bool | `true` |
| `Theme.balance` | float | `50` |
| `Theme.high-contrast` | bool | `false` |
| `Theme.reduce-motion` | bool | `false` |
| `Theme.reduce-transparency` | bool | `false` |
| `Theme.font-family` | string | `"Inter"` |
| `Theme.font-family-mono` | string | `"JetBrains Mono"` |
| `Theme.p` | float | `balance / 100` |
| **surfaces** | | |
| `Theme.bg` | color | `high-contrast ? (dark ? #000000 : #FFFFFF) : (dark ? #1D101C : #F6F0F6)` |
| `Theme.surface` | color | `high-contrast ? (dark ? #1C0F1A : #F3EAF4) : (dark ? #2A1929 : #FBF8FB)` |
| `Theme.raised` | color | `dark ? #33202F : #FFFFFF` |
| `Theme.inset` | color | `dark ? #160A14 : #F1ECF2` |
| `Theme.track` | color | `dark ? #3A2639 : #E8DCE9` |
| `Theme.glass` | color | `surface.with-alpha((reduce-transparency \|\| high-contrast) ? 1.0 : 0.79 + p * 0.05)` |
| **text** | | |
| `Theme.ink` | color | `high-contrast ? (dark ? #FFFFFF : #000000)` |
| `Theme.muted` | color | `high-contrast ? (dark ? #EBDDEA : #2A1226)` |
| `Theme.disabled-ink` | color | `ink.with-alpha(alpha-disabled)` |
| **lines** | | |
| `Theme.line` | color | `high-contrast ? ink : (dark ? #FFFFFF : #2A1226).with-alpha(0.06 + p * 0.12)` |
| `Theme.line-strong` | color | `high-contrast ? ink : (dark ? #FFFFFF : #2A1226).with-alpha(0.14 + p * 0.14)` |
| **accent (dusty orchid in dark, plum in light; chroma-capped, never neon)** | | |
| `Theme.accent` | color | `dark ? #E072C8.mix(#C883B0, p) : #722C5F.mix(#8A4275, p)` |
| `Theme.accent-hover` | color | `dark ? #E999D2 : accent.mix(#2A1226, 0.80)` |
| `Theme.accent-pressed` | color | `dark ? accent.mix(#000000, 0.86) : accent.mix(#2A1226, 0.62)` |
| `Theme.on-accent` | color | `dark ? #1A0817 : #FFFFFF` |
| `Theme.ring` | color | `dark ? #E999D2 : #7E376A` |
| `Theme.accent-wash` | color | `accent.with-alpha(dark ? 0.18 : 0.12)` |
| `Theme.accent-soft` | color | `accent.with-alpha(dark ? 0.28 : 0.20)` |
| **status** | | |
| `Theme.danger` | color | `dark ? #FF8A94 : #C0414E` |
| `Theme.danger-hover` | color | `dark ? danger.mix(#000000, 0.90) : danger.mix(#FFFFFF, 0.88)` |
| `Theme.danger-pressed` | color | `danger.mix(#000000, 0.80)` |
| `Theme.on-danger` | color | `dark ? #2A0A10 : #FFFFFF` |
| `Theme.success` | color | `dark ? #7BC74D : #3F7F1B` |
| `Theme.warning` | color | `dark ? #F2B04A : #9A5B00` |
| `Theme.leaf` | color | `dark ? #86AC6B : #2E451C` |
| `Theme.off` | color | `dark ? #9A8497 : #857082` |
| `Theme.scrim` | color | `#000000.with-alpha(0.48)` |
| **data series (charts, gauges): distinct hues that still belong to the palette** | | |
| `Theme.series-1` | color | `accent` |
| `Theme.series-2` | color | `dark ? #A592F2 : #5B4BB5` |
| `Theme.series-3` | color | `success` |
| `Theme.series-4` | color | `warning` |
| `Theme.series-5` | color | `dark ? #72C9D8 : #1F7F95` |
| `Theme.series-6` | color | `danger` |
| **brand constants (illustrations only; UI uses the roles above)** | | |
| `Theme.brand-petal` | color | `#E6DAE8` |
| `Theme.brand-lilac` | color | `#DEC3E0` |
| `Theme.brand-orchid` | color | `#B84F9F` |
| `Theme.brand-plum` | color | `#8C3274` |
| `Theme.brand-leaf` | color | `#274903` |
| **elevation (single drop shadow, plum-tinted; level 0 flat .. 4 modal)** | | |
| `Theme.shadow-tint` | color | `dark ? #000000 : #3C1432` |
| `Theme.shadow-1` | float | `0.04 + p * 0.12` |
| `Theme.shadow-2` | float | `0.08 + p * 0.20` |
| `Theme.shadow-3` | float | `0.14 + p * 0.20` |
| `Theme.shadow-4` | float | `0.18 + p * 0.24` |
| **space (4 pt grid)** | | |
| `Theme.space-xxs` | length | `4px` |
| `Theme.space-xs` | length | `8px` |
| `Theme.space-sm` | length | `12px` |
| `Theme.space-md` | length | `16px` |
| `Theme.space-lg` | length | `24px` |
| `Theme.space-xl` | length | `32px` |
| `Theme.space-xxl` | length | `48px` |
| **radii: controls 12 px and windows 20 px at balance 50; the dial moves both** | | |
| `Theme.radius-xs` | length | `4px` |
| `Theme.radius-sm` | length | `8px` |
| `Theme.radius-control` | length | `(8 + (100 - balance) * 0.08) * 1px` |
| `Theme.radius-card` | length | `radius-control + 4px` |
| `Theme.radius-window` | length | `(14 + (100 - balance) * 0.12) * 1px` |
| `Theme.radius-pill` | length | `999px` |
| **type: sizes are fixed, UI-label weight follows the dial (430 .. 570)** | | |
| `Theme.text-caption` | length | `11px` |
| `Theme.text-body-sm` | length | `13px` |
| `Theme.text-body` | length | `14px` |
| `Theme.text-subtitle` | length | `16px` |
| `Theme.text-title` | length | `20px` |
| `Theme.text-headline` | length | `28px` |
| `Theme.text-display` | length | `36px` |
| `Theme.weight-body` | int | `400` |
| `Theme.weight-ui` | int | `Math.round(430 + balance * 1.4)` |
| `Theme.weight-title` | int | `650` |
| **state layers and sizing** | | |
| `Theme.alpha-disabled` | float | `0.40` |
| `Theme.wash-hover` | float | `0.08` |
| `Theme.wash-pressed` | float | `0.14` |
| `Theme.press-scale` | float | `0.96` |
| `Theme.ring-width` | length | `2px` |
| `Theme.touch-target` | length | `44px` |
| `Theme.control-height` | length | `40px` |
| `Theme.control-height-sm` | length | `32px` |
| `Theme.control-height-lg` | length | `48px` |
| `Theme.dur-feedback` | duration | `reduce-motion ? 0ms : 120ms` |
| `Theme.dur-overlay` | duration | `reduce-motion ? 0ms : 240ms` |
| `Theme.dur-slow` | duration | `reduce-motion ? 0ms : 420ms` |
Vl