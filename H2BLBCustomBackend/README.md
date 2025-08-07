this follow the chapter 9 to 21 `H2BLB` backend creation

to use it you need :
* add the `H2BLB` to the target list
* put this folder in `llvm/lib/Target/H2BLB`
* add the H2BLB in the `TargetParser/Triple.cpp`
```cpp
static Triple::ObjectFormatType getDefaultFormat(const Triple &T) {
    switch (T.getArch()) {
        case Triple::h2blb:
            switch (T.getOS()) {
                case Triple::Win32:
                case Triple::UEFI:
                    return Triple::COFF;
        default:
            return T.isOSDarwin() ? Triple::MachO : Triple::ELF;
}

<snip>
unsigned Triple::getArchPointerBitWidth(llvm::Triple::ArchType Arch) {
    switch (Arch) {
        case llvm::Triple::h2blb:
            return 16;
<snip>

Triple Triple::get32BitArchVariant() const {
    Triple T(*this);
    switch (getArch()) {
        case llvm::Triple::h2blb:
            // This is already in 32-bit
            return T;
<snip>
```
