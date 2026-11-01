//! A window with the iced widgets in the Malva style, light and dark.
//!
//!     cargo run --release --example gallery        (in this directory)
//!
//! NOT COMPILED in the environment this kit was written in (it has no access to crates.io, so iced could not be
//! downloaded): it is written against the iced 0.14.0 source and follows its `styling` example, and the style
//! functions it uses are type-checked and tested against the stub in ../stub. Expect to fix a typo or two the first
//! time it meets the real iced.
use iced::widget::{
    button, checkbox, column, container, pick_list, progress_bar, radio, row, rule, scrollable, slider, space, svg, text,
    text_input, toggler, tooltip,
};
use iced::{Center, Element, Fill, Theme};
use malva_iced as malva;
use malva_tokens::Mode;

pub fn main() -> iced::Result {
    iced::application(Gallery::default, Gallery::update, Gallery::view)
        .theme(Gallery::theme)
        .style(Gallery::style)
        .font(malva::fonts::INTER_REGULAR)
        .font(malva::fonts::INTER_MEDIUM)
        .font(malva::fonts::INTER_SEMIBOLD)
        .font(malva::fonts::INTER_BOLD)
        .default_font(malva::fonts::UI)
        .run()
}

struct Gallery {
    dark: bool,
    name: String,
    volume: f32,
    wifi: bool,
    agree: bool,
    size: Option<Size>,
    flavour: Option<&'static str>,
    clicks: u32,
}

impl Default for Gallery {
    fn default() -> Self {
        Gallery { dark: false, name: String::new(), volume: 40.0, wifi: true, agree: false, size: Some(Size::Medium), flavour: None, clicks: 0 }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Size {
    Small,
    Medium,
    Large,
}

#[derive(Debug, Clone)]
enum Message {
    Dark(bool),
    Name(String),
    Volume(f32),
    Wifi(bool),
    Agree(bool),
    Size(Size),
    Flavour(&'static str),
    Clicked,
}

const FLAVOURS: [&str; 3] = ["Plum", "Orchid", "Lilac"];

impl Gallery {
    fn update(&mut self, message: Message) {
        match message {
            Message::Dark(v) => self.dark = v,
            Message::Name(v) => self.name = v,
            Message::Volume(v) => self.volume = v,
            Message::Wifi(v) => self.wifi = v,
            Message::Agree(v) => self.agree = v,
            Message::Size(v) => self.size = Some(v),
            Message::Flavour(v) => self.flavour = Some(v),
            Message::Clicked => self.clicks += 1,
        }
    }

    fn theme(&self) -> Theme {
        malva::theme(if self.dark { Mode::Dark } else { Mode::Light })
    }

    fn style(&self, theme: &Theme) -> iced::theme::Style {
        malva::app_style(theme)
    }

    fn view(&self) -> Element<'_, Message> {
        let icon = |name: &str| {
            svg(svg::Handle::from_memory(malva_icons::symbol(name).unwrap_or("").as_bytes()))
                .width(20)
                .height(20)
                .style(malva::text::icon)
        };
        let heading = |s: &'static str| text(s).size(20).font(malva::fonts::UI_SEMIBOLD);

        let header = row![
            text("Malva").size(28).font(malva::fonts::UI_BOLD),
            space().width(Fill),
            icon("moon"),
            toggler(self.dark).on_toggle(Message::Dark),
        ]
        .spacing(10)
        .align_y(Center);

        let buttons = row![
            button("Primary").on_press(Message::Clicked).style(malva::button::primary).padding([10, 18]),
            button("Secondary").on_press(Message::Clicked).style(malva::button::secondary).padding([10, 18]),
            button("Ghost").on_press(Message::Clicked).style(malva::button::ghost).padding([10, 18]),
            button("Delete").on_press(Message::Clicked).style(malva::button::danger).padding([10, 18]),
            button("Disabled").style(malva::button::primary).padding([10, 18]),
            tooltip(
                button(icon("star")).on_press(Message::Clicked).style(malva::button::icon).padding(10),
                container(text("Favourite").size(12)).padding([6, 10]).style(malva::container::tooltip),
                tooltip::Position::Bottom,
            ),
        ]
        .spacing(10)
        .align_y(Center);

        let controls = column![
            checkbox(self.agree).label("I agree").on_toggle(Message::Agree).style(malva::toggle::checkbox),
            toggler(self.wifi).label("Wi-Fi").on_toggle(Message::Wifi).style(malva::toggle::toggler).spacing(10),
            row![
                radio("Small", Size::Small, self.size, Message::Size).style(malva::toggle::radio),
                radio("Medium", Size::Medium, self.size, Message::Size).style(malva::toggle::radio),
                radio("Large", Size::Large, self.size, Message::Size).style(malva::toggle::radio),
            ]
            .spacing(16),
        ]
        .spacing(12);

        let inputs = column![
            text_input("Your name", &self.name).on_input(Message::Name).padding(10).style(malva::input::text_input),
            pick_list(FLAVOURS, self.flavour, Message::Flavour)
                .placeholder("Pick a colour")
                .padding(10)
                .style(malva::pick_list::default)
                .menu_style(malva::pick_list::menu),
        ]
        .spacing(12);

        let levels = column![
            row![text(format!("Volume {:.0} %", self.volume)).style(malva::text::secondary)],
            slider(0.0..=100.0, self.volume, Message::Volume).style(malva::slider::default),
            progress_bar(0.0..=100.0, self.volume).style(malva::progress::default).girth(8),
        ]
        .spacing(10);

        let card = container(column![heading("Settings"), controls, rule::horizontal(1).style(malva::rule::divider), inputs, levels].spacing(16))
            .padding(20)
            .width(Fill)
            .style(malva::container::card);

        let page = column![
            header,
            text(format!("Clicked {} times", self.clicks)).style(malva::text::muted),
            buttons,
            card,
        ]
        .spacing(20)
        .padding(24)
        .max_width(720);

        container(scrollable(container(page).center_x(Fill)).style(malva::scroll::default))
            .style(malva::container::background_layer)
            .into()
    }
}
