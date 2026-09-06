# Contributing to CSSv34 Client

First of all, thank you for considering contributing to CSSv34 Client.

This project is a community effort focused on making Counter-Strike: Source v34 cross-platform client using
[nillerusr's Source Engine](https://github.com/nillerusr/source-engine)
while preserving original game behavior wherever possible.

---

# Before You Start
### !! You should know the basics of working with the Source SDK. !!

Please:

- Read the README.md.
- Search existing Issues before opening a new one.
- Keep pull requests focused on one feature or fix.
- Test your changes before submitting them.

---

## Development Environment
#### The project currently targets Windows.

Recommended environment:

- Microsoft Visual Studio 2022 or 2026 (2022 requires additional changes)
- Windows 10 SDK (latest stable)
- MFC libraries installed
- v34 Dedicated server for multiplayer testing

## This may also be necessary
- [SDK for CSS v34](https://github.com/rusherr-c/cssv34-sdk)
- [Original Source 2007 Source code](https://github.com/uvbs/source-2007)
- [Original nillerusr's Source 2013 Source code](https://github.com/nillerusr/source-engine)

---

## Reporting Bugs

When reporting a bug, please include as much information as possible.

Useful information includes:

- Operating System
- Visual Studio version
- Windows SDK version
- Dedicated server version
- Client build being used
- Console output
- Crash logs
- Stack traces
- Screenshots (if applicable)

For crashes, include:

- exception code
- module name
- call stack

---

## Suggesting Features

Feature requests are welcome.

Examples include:

- Engine fixes
- CSS v34 compatibility improvements
- Networking improvements
- String table compatibility
- SendProp / RecvProp improvements
- VGUI improvements
- Physics fixes
- Rendering improvements
- Dedicated server improvements
- Tooling improvements

Please explain:

- what problem it solves
- why it is needed
- how it should work

---

## Coding Style

This project follows the general Source SDK coding style.

Guidelines:

- Use tabs for indentation where existing code uses tabs.
- Follow surrounding code style.
- Keep variable names descriptive.
- Avoid STL features and prefer using standard C libraries.
- Avoid unnecessary C++17/20 features unless required.
- Keep compatibility with the existing project.
- Prefer fixing the root cause instead of adding workarounds (if possible).
- Do not reformat unrelated files.

Example:

```cpp
void ExampleFunction()
{
    if ( !condition )
        return;

    DoSomething();
}
```

---

## Pull Requests

Please:

- Make one logical change per pull request.
- Write a meaningful title.
- Explain what changed.
- Explain why the change is needed.
- Mention any related Issue.

Large pull requests that mix unrelated changes may be requested to split.

---

## Testing

Before submitting a PR, verify that:

- Project builds successfully.
- Engine starts correctly.
- Client connects to a dedicated server.
- Existing functionality is not broken.
- No new compiler critical warnings are introduced.

If your change affects networking, please test:

- Sign-on sequence
- String Tables
- User Messages
- Net Messages

---

## Compatibility Policy

This project prioritizes compatibility with Counter-Strike: Source v34.

Changes that intentionally break compatibility with original v34 clients or servers are unlikely to be accepted unless there is a compelling technical reason.

---

## What NOT to Submit

Please avoid pull requests that only:

- Reformat unrelated code.
- Rename variables without functional changes.
- Replace tabs/spaces across entire files.
- Introduce large third-party libraries without discussion.
- Remove legacy code without verifying compatibility.

---

## Documentation

If your pull request changes user-visible behavior, please update documentation when appropriate.

This may include:

- README.md
- Troubleshooting section
- Build instructions
- Comments inside source code

---

## Security

Please do **not** publish security vulnerabilities as public issues.

Examples:

- Remote crashes
- Packet exploits
- Buffer overflows
- NetMessage vulnerabilities
- Engine memory corruption

Instead, contact the maintainer privately: [My Telegram](https://t.me/ent1tyname)

PLEASE DO NOT CONTACT ME WITHOUT PURPOSE

---

## Third-Party Code

When contributing code taken or adapted from another project:

- Preserve original copyright notices.
- Respect the original license.
- Clearly document the source.

---

## Goal of the Project

The primary goals of CSSv34 Client are:

- Improve compatibility with Counter-Strike: Source v34.
- Make CS:S v34 playable on Android devices
- Preserve original gameplay.
- Maintain compatibility with existing mods where possible.
- Improve engine stability.
- Modernize parts of the engine without breaking legacy behavior.

### Thank you for contributing!
