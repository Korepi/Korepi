import requests
import json
import re

# Reading api list
with open("map_api.txt", "r") as map_api_file:
    api_list = []
    for line in map_api_file.readlines():
        name, address = line.split()
        api_list.append((name, address))

# Getting map points data
points_messages = []
for (name, address) in api_list:
    r = requests.get(address)
    if r.status_code != 200:
        print(f"Failed load points data for {name}")
        continue

    points_messages.append((name, json.loads(r.content.decode())))

# Refactor map points data to our format
refactored_data = {}
for (name, message) in points_messages:
    label_list = message["data"]["label_list"]
    point_list = message["data"]["point_list"]

    label_map = {}
    for label in label_list:
        label_map[label["id"]] = label

    data = {}
    
    for point in point_list:
        label_id = point["label_id"]
        if label_id not in data:
            label = label_map[label_id]
            clear_name = re.sub(r"[^\w\d_]", "", label["name"])
            data[label_id] = {
                "name": label["name"],
                "clear_name": clear_name,
                "points": [],
                "icon": label["icon"]
            }

        points = data[label_id]["points"]
        points.append({
            "id": point["id"],
            "x_pos": point["x_pos"],
            "y_pos": point["y_pos"]
        })
        data[label_id]["points"] = sorted(points, key=lambda x: x["id"])
    

    refactored_data[name] = {
        "labels": dict(sorted(data.items())) # Sorting it so we can easily track changes in updates
    }

# Getting map categories data
categories_messages = []
for (name, address) in api_list:
    r = requests.get(address.replace("list", "tree").replace("point", "label"))
    if r.status_code != 200:
        print(f"Failed load categories data for {name}")
        continue

    categories_messages.append((name, json.loads(r.content.decode())))

# Refactor map categories data to our format
for (name, message) in categories_messages:
    categories_list = message["data"]["tree"]

    data = []
    for category in categories_list:
        data.append(
            {
                "id": category["id"],
                "name": category["name"],
                "children": sorted([child["id"] for child in category["children"]])
            }
        )
    
    refactored_data[name]["categories"] = sorted(data, key=lambda x: x["id"])

# Get ascension materials data
# They don't provide names for attrs so just manually set it...
character_types = {"1":"Pyro", "2":"Anemo", "3":"Geo", "4":"Dendro", "5":"Electro", "6":"Hydro", "7":"Cryo"}
weapon_types = {"1":"Swords", "10":"Catalysts", "11":"Claymores", "12":"Bows", "13":"Polearms"}

ascension_materials_messages = []
r = requests.get(api_list[0][1].replace("point/list", "game_item"))
if r.status_code != 200:
    print(f"Failed load ascension materials data")
else:
    ascension_materials_messages.append(("ascension_materials", json.loads(r.content.decode())))

# Refactor ascension materials data to our format
# Note: I've manually added the prefix "Character" to the clean_name in the generated json and image file of
# Amber and Tartaglia since they have conflicting clear_name with other labels icons.
# Entry for female Traveler is also removed since it's just a duplicate of the mc
material_types = { "avatar", "weapon" }
material_data = {}
for (name, message) in ascension_materials_messages:
    for type in material_types:
        material_list = message["data"][type]["list"]
        material_category_list = message["data"][type]["attrs"]
        data = {}
        for material in material_list:
            material_id = material["item_id"]
            clear_name = re.sub(r"[^\w\d_]", "", material["name"])
            data[material_id] = {
                "name": material["name"],
                "clear_name": clear_name,
                "materials": sorted(material["labels"]),
                "icon": material["icon"]
            }
        material_data[type] = dict(sorted(data.items()))

        data = []
        for category in material_category_list:
            children = []
            for material in material_list:
                if category == str(material["attr"]):
                    children.append(material["item_id"])
                    
            data.append(
                {
                "id": category,
                "name": character_types[category] if type == "avatar" else weapon_types[category],
                "children": sorted(children),
                "icon": material_category_list[category]["icon_chosen"]
                }
            )
        material_data[type + "_types"] = sorted(data, key=lambda x: x["id"])

    refactored_data[name] = {
        "character": material_data["avatar"],
        "character_types": material_data["avatar_types"],
        "weapon": material_data["weapon"],
        "weapon_types": material_data["weapon_types"]
    }

# Writing refactored data to json files
for (name, data) in refactored_data.items():
    with open(f"../{name}.json", "w") as out_json_file:
        json.dump(data, out_json_file, indent=4)
