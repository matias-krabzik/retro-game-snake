# Downloads

Pre-built binaries for Retro Snake. No dependencies required — just download and play.

## macOS

| Platform | Architecture | Download |
|----------|-------------|----------|
| macOS (Sequoia+) | Apple Silicon (M1/M2/M3/M4) | [Snake.app](https://github.com/matias-krabzik/retro-game-snake/raw/main/downloads/Snake.app.zip) |

### Instructions

1. Download `Snake.app.zip`
2. Unzip and move `Snake.app` to your Applications folder (or anywhere you like)
3. Before first launch, remove the quarantine attribute:
   ```bash
   xattr -cr ~/Downloads/Snake.app
   ```
4. Double-click to play. If macOS still blocks it, right-click the app and select **Open**, then click **Open** again in the dialog

## Building from source

If your platform isn't listed above, you can build from source. See the [main README](../README.md#building) for instructions.
