# machash - hash strings into MAC addresses.
#
# The binary is a single-file static universal binary built with
# cosmocc. The cosmocc helper tools are Cosmopolitan (APE)
# binaries. A build host has no APE binfmt handler, so each helper
# is run through the APE loader for this architecture. At install
# time the APE handler is registered so the binary can run on the
# host.
#
# The test suite runs in the upstream CI. It does not run in the
# build chroot, which has no APE binfmt handler.

Name:           machash
Version:        1.0.2
Release:        1
Summary:        Hash arbitrary strings into MAC addresses

License:        BSD-3-Clause
URL:            https://github.com/MrDrMcCoy/machash
Source0:        %{url}/releases/download/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  curl
BuildRequires:  unzip

%description
machash hashes input strings with the 48-bit Bobcat hash and prints
the result as a colon-separated MAC address. The address is fake,
but it has the form of a valid MAC address. This is useful for
deriving a stable, deterministic MAC address from any string.

%prep
%setup -q

%build
curl -fsSL -o cosmocc.zip \
  "https://cosmo.zip/pub/cosmocc/cosmocc-4.0.2.zip"
mkdir -p cosmo
unzip -q cosmocc.zip -d cosmo
for t in apelink fixupobj pecheck mktemper \
  x86_64-linux-cosmo-gcc aarch64-linux-cosmo-gcc; do
  mv "cosmo/bin/$t" "cosmo/bin/$t.ape"
  {
    printf '#!/bin/sh\n'
    printf 'exec "$(dirname "$0")/ape-%{?_target_cpu}.elf" '
    printf '"$(dirname "$0")/%s.ape" "$@"\n' "$t"
  } > "cosmo/bin/$t"
  chmod +x "cosmo/bin/$t"
done
make build CC="$PWD/cosmo/bin/cosmocc"

%install
install -Dpm0755 dist/machash %{buildroot}%{_bindir}/machash
install -Dpm0755 "cosmo/bin/ape-%{?_target_cpu}.elf" \
  %{buildroot}%{_libdir}/machash/ape
install -Dpm0644 man/machash.1 %{buildroot}%{_mandir}/man1/machash.1
install -Dpm0644 LICENSE %{buildroot}%{_docdir}/machash/LICENSE
install -Dpm0644 readme.md %{buildroot}%{_docdir}/machash/readme.md
install -Dpm0644 changelog.md dependencies.md \
  %{buildroot}%{_docdir}/machash/
install -Dpm0644 docs/*.md %{buildroot}%{_docdir}/machash/

%post
# Register the APE binfmt handler if it is not present.
if [ ! -e /proc/sys/fs/binfmt_misc/APE ]; then
  if [ ! -d /proc/sys/fs/binfmt_misc ]; then
    mount -t binfmt_misc none /proc/sys/fs/binfmt_misc 2>/dev/null || true
  fi
  if [ -d /proc/sys/fs/binfmt_misc ]; then
    echo ':APE:M::MZqFpD::%{_libdir}/machash/ape:' \
      > /proc/sys/fs/binfmt_misc/register
  else
    echo "warning: could not register the APE handler; " \
      "machash needs it to run" >&2
  fi
fi
exit 0

%postun
# Unregister the APE handler if this package registered it.
if [ "$1" = "0" ] && [ -r /proc/sys/fs/binfmt_misc/APE ]; then
  interp=$(awk -F: '{print $NF}' /proc/sys/fs/binfmt_misc/APE)
  if [ "$interp" = "%{_libdir}/machash/ape" ]; then
    echo '-APE' > /proc/sys/fs/binfmt_misc/register
  fi
fi
exit 0

%files
%doc %{_docdir}/machash
%{_bindir}/machash
%{_libdir}/machash/ape
%{_mandir}/man1/machash.1*
