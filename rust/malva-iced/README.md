# malva-iced

The Malva theme for [iced](https://iced.rs) 0.14: one `Theme` per mode and plain style functions for iced's widgets.

```rust
iced::application(State::default, State::update, State::view)
    .theme(|s: &State| malva_iced::theme(s.mode))
    .style(|_, theme| malva_iced::app_style(theme))
    .font(malva_iced::fonts::INTER_REGULAR)
    .default_font(malva_iced::fonts::UI)
    .run()
```

`button("Save").style(malva_iced::button::primary)`, `container(x).style(malva_iced::container::card)`, and the same
for `text_input`, `text_editor`, `checkbox`, `radio`, `toggler`, `slider`, `progress_bar`, `scrollable`, `pick_list`,
`rule`, `text`, `svg`. Colours come from `malva-tokens`, so the same numbers drive the Slint, C++, CSS and iced sides.

## What is tested, and what is not

| | |
|---|---|
| The style functions return the right token for every status, light and dark (34 tests) | **yes, against a stub** |
| The style functions type-check against iced's real style structs | **yes**: the stub's structs are extracted verbatim from the iced 0.14.0 source |
| The crate builds and runs against the real iced from crates.io | **not tested** (no access to crates.io where it was written) |
| `examples/gallery.rs` compiles and draws | **not tested** |
| How it looks on screen | **not tested**: the look is the Slint rendering of the same tokens (docs/screenshots) |

Run the stub tests yourself (needs a checkout of iced 0.14.0 for the struct definitions, nothing else):

```sh
git clone --depth 1 --branch 0.14.0 https://github.com/iced-rs/iced /tmp/iced
python3 stub/make_stub.py /tmp/iced /tmp/iced-stub
cargo --config 'patch.crates-io.iced.path="/tmp/iced-stub"' test --lib --offline
```

Against real iced: `cargo test` here, and `cargo run --release --example gallery`.
