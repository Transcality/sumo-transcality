import os
import shutil

# Function to copy a file if a reference file exists in the folder


def copy_file_if_reference_exists(source_file, base_folder, reference_file):
    # Check if the source file exists
    if not os.path.isfile(source_file):
        print(f"The source file '{source_file}' does not exist.")
        return

    # Traverse all folders and subfolders in the base folder
    for root, dirs, files in os.walk(base_folder):
        for dir_name in dirs:
            destination_folder = os.path.join(root, dir_name)
            reference_file_path = os.path.join(destination_folder, reference_file)

            # Check if the reference file exists in the folder
            if os.path.isfile(reference_file_path):
                try:
                    # Copy the file to the current folder
                    shutil.copy(source_file, destination_folder)
                    print(f"File copied to: {destination_folder}")
                except Exception as e:
                    print(f"Could not copy the file to '{destination_folder}': {e}")


# Main block
if __name__ == "__main__":

    # Path of the file to be copied
    source_file = r"D:\meandatas.netedit"

    # Path of the base folder where the file will be copied
    base_folder = r"D:\SUMO"

    # Name of the reference file that must exist in the folder
    reference_file = "routes.netedit"

    # Call the function
    copy_file_if_reference_exists(source_file, base_folder, reference_file)