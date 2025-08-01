import re

# Read the file
with open(r'c:\training\kamatech\KungFuChess-New\KungFuChess\src\Game.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Remove all add_move_to_history calls
content = re.sub(r'\s*add_move_to_history\([^)]+\);\s*', '', content)

# Add timestamp to piece_moved events that don't have it
pattern = r'(eventData\["to"\] = [^;]+;\s*)(eventPublisher_\.publish\(GameEvent\("piece_moved", eventData\)\);)'
replacement = r'\1eventData["timestamp"] = std::to_string(game_time_ms());\r\n                            \2'
content = re.sub(pattern, replacement, content)

# Write back
with open(r'c:\training\kamatech\KungFuChess-New\KungFuChess\src\Game.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Fixed Game.cpp")