{
  pkgs ? import <nixpkgs> {}
, debug ? true
}:

let
  name = "kj-xml";

  create-ekam-rules-link = ''
    ln --symbolic --force --target-directory=src \
      "${pkgs.ekam.src}/src/ekam/rules"
  '';

in

pkgs.stdenv.mkDerivation {

  inherit name;
  src = ./.;

  buildInputs = with pkgs; [
    capnproto
    openssl
  ];

  nativeBuildInputs = with pkgs; [
    ccache
    clang-tools
    ekam
    gtest
    which
  ];

  propagatedBuildInputs = with pkgs; [
  ];

  CAPNPC_FLAGS = with pkgs; [
    "-I${capnproto}/include"
  ];

  shellHook = create-ekam-rules-link;

  buildPhase = ''
    ${create-ekam-rules-link}
    make ${if debug then "debug" else "release"}
  '';

  # Pointless for static libraries, but uncomment if we ever move to a shared
  # object
  #separateDebugInfo = true;

  installPhase = ''
    install --verbose -D --mode=644 \
      --target-directory="''${!outputLib}/lib" \
      lib${name}.a

    install --verbose -D --mode=644 \
      --target-directory="''${!outputInclude}/include/${name}" \
      src/*.capnp \
      src/*.capnp.h \
      tmp/*.capnp.h \
      src/*.h 
  '';
}
