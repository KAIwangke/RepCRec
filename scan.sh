#!/bin/bash
# Output file
OUTPUT_FILE="all_contents.txt"

# Clear or create output file
> "$OUTPUT_FILE"

# Add header with timestamp
echo "Directory Contents Dump - $(date)" >> "$OUTPUT_FILE"
echo "Current Directory: $(pwd)" >> "$OUTPUT_FILE"
echo "=========================================" >> "$OUTPUT_FILE"
echo >> "$OUTPUT_FILE"

# Find all files while ignoring .git, build dir, and binary files
find . -type f \
    -not -path "*.git*" \
    -not -path "*build*" \
    -not -name "*.o" \
    -not -name "*.exe" \
    -not -name "*.a" \
    -not -name "*.so" \
    -not -name "*.pyc" \
    -not -name "*.class" \
    -not -name "*.jar" \
    -not -name "*.war" \
    -not -name "*.dll" \
    -not -name "*.bin" \
    -print0 | while IFS= read -r -d '' file; do
    
    # Print file header
    echo "=== File: $file ===" >> "$OUTPUT_FILE"
    echo "----------------------------------------" >> "$OUTPUT_FILE"
    
    # Try to cat the file contents, redirect errors to /dev/null
    cat "$file" 2>/dev/null >> "$OUTPUT_FILE"
    
    # Add spacer between files
    echo -e "\n\n" >> "$OUTPUT_FILE"
done

echo "Contents have been saved to $OUTPUT_FILE"