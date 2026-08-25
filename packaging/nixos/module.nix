# NixOS module for machash.
#
# Installs the package and registers the APE binfmt handler, so the
# universal binary runs without further setup. Registration is the
# NixOS equivalent of the postinst scripts of the other packages.
#
# Use it like this, in configuration.nix:
#
#   imports = [ /path/to/packaging/nixos/module.nix ];
#
#   machash.enable = true;

{ config, lib, pkgs, ... }:

let
  machash = pkgs.callPackage ./default.nix { };
in
{
  options.machash.enable = lib.mkEnableOption
    "machash, the tool that hashes strings into MAC addresses";

  config = lib.mkIf config.machash.enable {
    environment.systemPackages = [ machash ];
    boot.binfmt.registrations.APE = {
      interpreter = "${machash}/lib/machash/ape";
      magic = "MZqFpD";
    };
  };
}
