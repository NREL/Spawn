# Spawn EnergyPlus Coroutine Input

This directory contains code to interface with the primary Spawn input json file.
The main point of entry is the `Input` class. 
There are currently a number of different development patterns implemented and it would
be good to improve this by consistently implementing nlohmann's `to_json`, `from_json`
patterns https://github.com/nlohmann/json#basic-usage.
