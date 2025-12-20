{
  description = "EZLive: Self-hosted Serverless Livestream";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "ezlive";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          buildInputs = with pkgs; [
            ffmpeg
            srt
            aws-sdk-cpp
          ];

          makeFlags = [
            "CC=${pkgs.stdenv.cc}/bin/cc"
            "CXX=${pkgs.stdenv.cc}/bin/c++"
          ];

          installPhase = ''
            runHook preInstall
            mkdir -p $out/bin
            cp ezlive $out/bin/
            runHook postInstall
          '';

          meta = with pkgs.lib; {
            description = "Self-hosted Serverless Livestream built on top of S3-compatible object storage";
            homepage = "https://github.com/mistivia/ezlive";
            license = licenses.agpl3Only;
            maintainers = [ ];
            platforms = platforms.linux;
          };
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            gcc
            gnumake
            pkg-config
            ffmpeg
            aws-sdk-cpp
            srt
          ];

          shellHook = ''
            echo "EZLive development environment"
            echo "Run 'make' to build the project"
          '';
        };

        packages.docker = pkgs.dockerTools.buildLayeredImage {
          name = "ezlive";
          tag = "latest";
          
          contents = [ self.packages.${system}.default ];
          
          config = {
            Cmd = [ "${self.packages.${system}.default}/bin/ezlive" ];
            WorkingDir = "/app";
          };
        };
      }
    );
}
