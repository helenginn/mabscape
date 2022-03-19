"""Script to clean up competition data for use in mabscape."""

def clean_name(name:str) -> str:
    new_name = "ab"
    if len(name) >= 3:
        new_name += name
    elif len(name) == 2:
        new_name += "0" + name
    elif len(name) == 1:
        new_name += "00" + name

    return new_name


def calculate_competition(measured:list) -> str:
    try:
        competition = 1 - float(measured[2])
    except:
        print("Competition for the following line could not be calculated and was skipped : " + str(measured))
        competition = measured[2]
    return competition


def main():
    input_file_name = input("Please choose input file:")
    output_file_name = input("Please choose output file:")

    lines = []
    with open(input_file_name) as input_file:
        for line in input_file:
            lines.append([line.split(",")[0],line.split(",")[1],line.split(",")[2].split("\n")[0]])
            lines[-1][0] = clean_name(lines[-1][0])
            lines[-1][1] = clean_name(lines[-1][1])
            lines[-1][2] = calculate_competition(lines[-1])
    
    with open(output_file_name,"w+") as output:
        for line in lines:
            output.write(str(line[0]) + "," +  str(line[1]) + "," + str(line[2]) + "\n")

if __name__ == "__main__":
    main()
