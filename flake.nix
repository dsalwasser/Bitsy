{
  description = "An itsy-bitsy bit vector with rank and select support.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };

      inputs = builtins.attrValues {
        inherit (pkgs) cmake ninja gcc14 python312;
      };

      devShellInputs = builtins.attrValues {
        inherit (pkgs) fish ccache gdb;
        inherit (pkgs.linuxPackages_latest) perf;
      };
    in
    {
      devShells = rec {
        default = gcc;

        gcc = pkgs.mkShell {
          packages = inputs ++ devShellInputs;

          shellHook = ''
            exec fish
          '';
        };

        clang = (pkgs.mkShell.override { stdenv = pkgs.llvmPackages_18.stdenv; }) {
          packages = (pkgs.lib.lists.remove pkgs.gcc14 inputs) ++ devShellInputs;

          shellHook = ''
            exec fish
          '';
        };
      };
    }
  );
}
