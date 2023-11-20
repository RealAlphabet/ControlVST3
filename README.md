# Before use

## Warning

⚠️ **THIS PROJECT IS HIGHLY EXPERIMENTAL AND IS NOT INTENDED FOR PERSONAL USE FOR THE TIME BEING**.  
Although in the worst case your system should not be impacted, the project is far from finished and is still in the prototype stage.

## Compatibility

Tested on FL Studio 20 only on Windows 11.
> ⚠️ **Only work on Windows at the moment.**

# How to use

## Installation
- Add the bot [`Control`](https://discord.com/api/oauth2/authorize?client_id=1168331621519331389&permissions=4332047681&scope=applications.commands%20bot) on the server.
- [Download the artifact to the Github artifact.](https://github.com/RealAlphabet/ControlVST3/actions/runs/6832154448)
- Extract the contents of the archive into the `C:/Common Files/VST3/` folder or another location where you have your plugins.

## How to use

1. Join a voice channel.
2. Write `/live` to join the bot.
3. Add the `Control Master` plugin as the last effect on a mixer track.  
  **Tip:** Add it to the Master track.
4. Play or play an instrument.

> **Note:** Even if the mixer track is muted or the volume is turned down, the bot will still play the audio at the same volume as the track's input volume.

# Future work

Currently, no authentication mechanism is available and you cannot change the hardcoded server IP address from the VST UI.
